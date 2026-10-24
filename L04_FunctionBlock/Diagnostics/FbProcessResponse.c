#include "FbProcessResponse.h"
#include <stddef.h>

void FbProcessResponse_Init(FbProcessResponse *s)
{
    if (s!=NULL) *s=(FbProcessResponse){0};
}
static uint64_t Magnitude(int64_t x) { return (uint64_t)(x<0 ? -x : x); }
bool FbProcessResponse_ValidateConfig(const FbProcessResponseConfig *c)
{
    return c!=NULL && c->minimum_mv_step>0 && c->minimum_pv_span>0 &&
        c->onset_change>0 && c->minimum_settling_samples>=2 &&
        c->timeout_ms>0 && c->settling_dwell_ms<=c->timeout_ms;
}
static bool Same(const FbProcessResponseConfig *a, const FbProcessResponseConfig *b)
{
    return a->final_pv==b->final_pv && a->step_mv==b->step_mv &&
        a->mv_tolerance==b->mv_tolerance && a->minimum_mv_step==b->minimum_mv_step &&
        a->minimum_pv_span==b->minimum_pv_span && a->onset_change==b->onset_change &&
        a->settling_band==b->settling_band && a->settling_dwell_ms==b->settling_dwell_ms &&
        a->minimum_settling_samples==b->minimum_settling_samples && a->timeout_ms==b->timeout_ms;
}
static FbResponseStatus Publish(FbProcessResponse *s, FbProcessResponseOutput *o, FbResponseStatus status)
{
    *o=s->metrics; o->status=status; return status;
}
static FbResponseStatus Fault(FbProcessResponse *s, FbProcessResponseOutput *o, FbResponseStatus status)
{
    s->metrics.phase=FB_RESPONSE_FAULT; s->metrics.result_valid=false;
    s->metrics.status=status; return Publish(s,o,status);
}
FbResponseStatus FbProcessResponse_Execute(FbProcessResponse *s,
    const FbProcessResponseConfig *c, const FbProcessResponseInput *in, FbProcessResponseOutput *o)
{
    bool edge, start=false;
    int64_t progress, final_error;
    uint64_t span;
    if (o==NULL) return FB_RESPONSE_INVALID_ARGUMENT;
    *o=(FbProcessResponseOutput){0}; o->status=FB_RESPONSE_INVALID_ARGUMENT;
    if (s==NULL || c==NULL || in==NULL) return o->status;
    if (!in->enable) {
        FbProcessResponse_Init(s); o->status=FB_RESPONSE_DISABLED; return o->status;
    }
    edge=in->command!=FB_RESPONSE_CMD_NONE && in->command!=s->previous_command;
    s->previous_command=in->command;
    if (edge && in->command==FB_RESPONSE_CMD_RESET) {
        FbProcessResponse_Init(s); s->previous_command=in->command;
        return Publish(s,o,FB_RESPONSE_OK);
    }
    if (s->metrics.phase==FB_RESPONSE_FAULT) return Publish(s,o,s->metrics.status);
    if (in->command<FB_RESPONSE_CMD_NONE || in->command>FB_RESPONSE_CMD_RESET)
        return Fault(s,o,FB_RESPONSE_INVALID_INPUT);
    if (!FbProcessResponse_ValidateConfig(c) ||
        (s->metrics.phase==FB_RESPONSE_OBSERVING && !Same(&s->config,c)))
        return Fault(s,o,FB_RESPONSE_INVALID_CONFIG);
    if (edge && in->command==FB_RESPONSE_CMD_STOP && s->metrics.phase==FB_RESPONSE_OBSERVING) {
        s->metrics.phase=FB_RESPONSE_CANCELLED;
        return Publish(s,o,FB_RESPONSE_OK);
    }
    if (edge && in->command==FB_RESPONSE_CMD_START && s->metrics.phase!=FB_RESPONSE_OBSERVING) {
        int64_t mv_step=(int64_t)c->step_mv-in->baseline_mv;
        int64_t pv_span=(int64_t)c->final_pv-in->baseline_pv;
        if (!in->baseline_valid || !in->valid || Magnitude(mv_step)<c->minimum_mv_step ||
            Magnitude(pv_span)<c->minimum_pv_span || c->onset_change>Magnitude(pv_span) ||
            (uint64_t)c->settling_band*1000>=Magnitude(pv_span)*100)
            return Fault(s,o,FB_RESPONSE_INVALID_INPUT);
        /* Settling band must be narrower than 10% of reference change, so
         * a flat baseline cannot masquerade as a settled response. */
        FbProcessResponse_Init(s); s->previous_command=in->command;
        s->config=*c; s->baseline_pv=in->baseline_pv;
        s->mv_step=mv_step; s->pv_span=pv_span;
        s->metrics.reference_gain=(double)pv_span/(double)mv_step;
        s->metrics.phase=FB_RESPONSE_OBSERVING; start=true;
    }
    if (s->metrics.phase!=FB_RESPONSE_OBSERVING) return Publish(s,o,FB_RESPONSE_OK);
    if (!in->valid) return Fault(s,o,FB_RESPONSE_INVALID_INPUT);
    if (Magnitude((int64_t)in->mv-c->step_mv)>c->mv_tolerance)
        return Fault(s,o,FB_RESPONSE_EXPERIMENT_CHANGED);
    s->metrics.elapsed_ms+=start ? 0 : in->elapsed_ms;
    /* Active runtime never exceeds uint32 timeout before this call. */
    if (s->metrics.elapsed_ms>c->timeout_ms) return Fault(s,o,FB_RESPONSE_TIMEOUT);
    if (!s->has_sequence || s->previous_sequence!=in->sample_sequence) {
        if (s->metrics.samples==UINT64_MAX)
            return Fault(s,o,FB_RESPONSE_COUNTER_OVERFLOW);
        s->has_sequence=true; s->previous_sequence=in->sample_sequence;
        ++s->metrics.samples;
        progress=(int64_t)in->pv-s->baseline_pv;
        if (s->pv_span<0) progress=-progress;
        span=Magnitude(s->pv_span);
        if (progress>=(int64_t)c->onset_change && !s->metrics.onset_seen) {
            s->metrics.onset_seen=true; s->metrics.onset_ms=s->metrics.elapsed_ms;
        }
        if (progress>=0) {
            uint64_t scaled=(uint64_t)progress*1000;
            if (scaled>=span*100 && !s->metrics.t10_seen) {
                s->metrics.t10_seen=true; s->metrics.t10_ms=s->metrics.elapsed_ms;
            }
            if (scaled>=span*632 && !s->metrics.t632_seen) {
                s->metrics.t632_seen=true; s->metrics.t632_ms=s->metrics.elapsed_ms;
            }
            if (scaled>=span*900 && !s->metrics.t90_seen) {
                s->metrics.t90_seen=true; s->metrics.t90_ms=s->metrics.elapsed_ms;
            }
            if ((uint64_t)progress>span && (uint64_t)progress-span>s->metrics.overshoot)
                s->metrics.overshoot=(uint64_t)progress-span;
        }
        final_error=(int64_t)in->pv-c->final_pv;
        if (Magnitude(final_error)<=c->settling_band) {
            if (!s->in_band) {
                s->in_band=true; s->band_start_ms=s->metrics.elapsed_ms;
                s->band_samples=0; s->band_mean=0;
            }
            ++s->band_samples;
            s->band_mean+=((double)in->pv-s->band_mean)/(double)s->band_samples;
            if (s->metrics.elapsed_ms-s->band_start_ms>=c->settling_dwell_ms &&
                s->band_samples>=c->minimum_settling_samples) {
                s->metrics.phase=FB_RESPONSE_COMPLETED;
                s->metrics.result_valid=true;
                s->metrics.settling_ms=s->metrics.elapsed_ms;
                s->metrics.final_mean=s->band_mean;
                s->metrics.measured_gain=(s->band_mean-s->baseline_pv)/(double)s->mv_step;
            }
        } else { s->in_band=false; s->band_samples=0; s->band_mean=0; }
    }
    if (s->metrics.phase==FB_RESPONSE_OBSERVING && s->metrics.elapsed_ms>=c->timeout_ms)
        return Fault(s,o,FB_RESPONSE_TIMEOUT);
    return Publish(s,o,FB_RESPONSE_OK);
}
