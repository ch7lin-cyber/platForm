#include "FbBatchMetrics.h"
#include <stddef.h>

void FbBatchMetrics_Init(FbBatchMetrics *s)
{
    if (s!=NULL) *s=(FbBatchMetrics){0};
}
static bool Same(const FbBatchMetricsConfig *a, const FbBatchMetricsConfig *b)
{
    return a->arrival_band==b->arrival_band && a->arrival_dwell_ms==b->arrival_dwell_ms &&
        a->direction==b->direction;
}
static FbBatchStatus Publish(FbBatchMetrics *s, FbBatchMetricsOutput *o, FbBatchStatus status)
{
    *o=s->metrics; o->status=status;
    o->valid=s->metrics.samples!=0 && s->metrics.phase!=FB_BATCH_FAULT;
    o->population_variance=s->metrics.samples ? s->m2/(double)s->metrics.samples : 0;
    return status;
}
FbBatchStatus FbBatchMetrics_Execute(FbBatchMetrics *s,
    const FbBatchMetricsConfig *c, const FbBatchMetricsInput *in, FbBatchMetricsOutput *o)
{
    bool edge, start=false, new_sample, in_band;
    uint32_t dt;
    int64_t error;
    uint64_t magnitude, directional;
    if (o==NULL) return FB_BATCH_INVALID_ARGUMENT;
    *o=(FbBatchMetricsOutput){0}; o->status=FB_BATCH_INVALID_ARGUMENT;
    if (s==NULL || c==NULL || in==NULL) return o->status;
    if (!in->enable) {
        FbBatchMetrics_Init(s); o->status=FB_BATCH_DISABLED; return o->status;
    }
    edge=in->command!=FB_BATCH_CMD_NONE && in->command!=s->previous_command;
    s->previous_command=in->command;
    if (edge && in->command==FB_BATCH_CMD_RESET) {
        FbBatchMetrics_Init(s); s->previous_command=in->command;
        return Publish(s,o,FB_BATCH_OK);
    }
    if (s->metrics.phase==FB_BATCH_FAULT) return Publish(s,o,s->metrics.status);
    if ((c->direction!=FB_BATCH_HEATING && c->direction!=FB_BATCH_COOLING) ||
        (s->metrics.phase==FB_BATCH_RUNNING && !Same(&s->config,c))) {
        s->metrics.phase=FB_BATCH_FAULT; s->metrics.status=FB_BATCH_INVALID_CONFIG;
        return Publish(s,o,FB_BATCH_INVALID_CONFIG);
    }
    if (in->command<FB_BATCH_CMD_NONE || in->command>FB_BATCH_CMD_RESET) {
        s->metrics.phase=FB_BATCH_FAULT; s->metrics.status=FB_BATCH_INVALID_INPUT;
        return Publish(s,o,FB_BATCH_INVALID_INPUT);
    }
    if (edge && in->command==FB_BATCH_CMD_STOP && s->metrics.phase==FB_BATCH_RUNNING) {
        s->metrics.phase=FB_BATCH_COMPLETED; s->metrics.finished=true;
        return Publish(s,o,FB_BATCH_OK);
    }
    if (edge && in->command==FB_BATCH_CMD_START && s->metrics.phase!=FB_BATCH_RUNNING) {
        if (!in->sv_valid) return Publish(s,o,FB_BATCH_INVALID_INPUT);
        FbBatchMetrics_Init(s); s->previous_command=in->command;
        s->config=*c; s->metrics.target=in->sv; s->metrics.phase=FB_BATCH_RUNNING;
        start=true;
    }
    if (s->metrics.phase!=FB_BATCH_RUNNING) return Publish(s,o,FB_BATCH_OK);
    dt=start ? 0 : in->elapsed_ms;
    new_sample=!s->has_sequence || in->sample_sequence!=s->previous_sequence;
    if (UINT64_MAX-s->metrics.elapsed_ms<dt ||
        (!in->pv_valid && s->metrics.invalid_calls==UINT64_MAX) ||
        (in->pv_valid && new_sample && s->metrics.samples==UINT64_MAX)) {
        s->metrics.phase=FB_BATCH_FAULT; s->metrics.status=FB_BATCH_COUNTER_OVERFLOW;
        return Publish(s,o,FB_BATCH_COUNTER_OVERFLOW);
    }
    s->metrics.elapsed_ms+=dt;
    if (!in->pv_valid) {
        ++s->metrics.invalid_calls;
        s->previous_valid=false; s->previous_in_band=false; s->dwell_ms=0;
        return Publish(s,o,FB_BATCH_INVALID_INPUT);
    }
    error=(int64_t)in->pv-s->metrics.target;
    magnitude=(uint64_t)(error<0 ? -error : error);
    in_band=magnitude<=c->arrival_band;
    if (new_sample) {
        double delta;
        ++s->metrics.samples;
        if (s->metrics.samples==1 || in->pv<s->metrics.minimum) s->metrics.minimum=in->pv;
        if (s->metrics.samples==1 || in->pv>s->metrics.maximum) s->metrics.maximum=in->pv;
        delta=(double)in->pv-s->metrics.mean;
        s->metrics.mean+=delta/(double)s->metrics.samples;
        s->m2+=delta*((double)in->pv-s->metrics.mean);
        s->has_sequence=true; s->previous_sequence=in->sample_sequence;
    }
    directional=(c->direction==FB_BATCH_HEATING ? error>0 : error<0) ? magnitude : 0;
    if (directional>s->metrics.overshoot) s->metrics.overshoot=directional;
    if (s->previous_valid) {
        int64_t previous_error=(int64_t)s->previous_pv-s->metrics.target;
        uint64_t absolute=(uint64_t)(previous_error<0 ? -previous_error : previous_error);
        /* Left rectangle: preceding valid value covers this validated interval. */
        s->metrics.signed_error_integral+=(double)previous_error*((double)dt/1000);
        s->metrics.absolute_error_integral+=(double)absolute*((double)dt/1000);
        s->metrics.integrated_ms+=dt; /* <= elapsed_ms, overflow checked above */
    }
    if (!s->metrics.reached) {
        if (in_band && s->previous_valid && s->previous_in_band) s->dwell_ms+=dt;
        else s->dwell_ms=0;
        if (in_band && s->dwell_ms>=c->arrival_dwell_ms) {
            s->metrics.reached=true; s->metrics.arrival_ms=s->metrics.elapsed_ms;
        }
    }
    s->previous_pv=in->pv; s->previous_valid=true; s->previous_in_band=in_band;
    return Publish(s,o,FB_BATCH_OK);
}
