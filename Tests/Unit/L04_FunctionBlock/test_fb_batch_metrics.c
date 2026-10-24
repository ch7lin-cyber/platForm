#include "FbBatchMetrics.h"
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
int main(void)
{
    FbBatchMetrics s={0}, other={0};
    FbBatchMetricsConfig c={100,1000,FB_BATCH_HEATING};
    FbBatchMetricsInput in={900,1000,1,999,true,true,true,FB_BATCH_CMD_START};
    FbBatchMetricsOutput o;
#define RUN() FbBatchMetrics_Execute(&s,&c,&in,&o)
#define COMMAND(x) do {in.command=(x); RUN(); in.command=FB_BATCH_CMD_NONE;} while(0)
    assert(RUN()==FB_BATCH_OK && o.samples==1 && o.elapsed_ms==0 && !o.reached);
    in.command=FB_BATCH_CMD_NONE; in.elapsed_ms=500;
    RUN(); assert(o.samples==1 && o.elapsed_ms==500 && !o.reached);
    assert(o.integrated_ms==500 && o.absolute_error_integral==50);
    ++in.sample_sequence; in.pv=1100; RUN();
    assert(o.samples==2 && o.mean==1000 && o.minimum==900 && o.maximum==1100);
    assert(o.population_variance==10000 && o.overshoot==100);
    assert(o.reached && o.arrival_ms==1000 && o.absolute_error_integral==100);
    assert(o.signed_error_integral==-100);
    /* Incoming SV does not move captured target mid-batch. */
    in.sv=999999; in.sv_valid=false; RUN(); assert(o.target==1000);
    /* Invalid data retains historical metrics, no integration across gaps. */
    in.pv_valid=false; in.elapsed_ms=1000;
    assert(RUN()==FB_BATCH_INVALID_INPUT && o.invalid_calls==1 && o.valid);
    assert(o.integrated_ms==1500);
    in.pv_valid=true; ++in.sample_sequence; in.pv=1000;
    RUN(); assert(o.integrated_ms==1500 && o.samples==3);
    RUN(); assert(o.integrated_ms==2500 && o.samples==3);
    COMMAND(FB_BATCH_CMD_STOP); assert(o.phase==FB_BATCH_COMPLETED && o.finished);
    uint64_t elapsed=o.elapsed_ms; RUN(); assert(o.elapsed_ms==elapsed);
    /* RESET clears; restart requires valid SV, held START doesn't restart. */
    COMMAND(FB_BATCH_CMD_RESET); assert(!o.valid && o.samples==0);
    in.command=FB_BATCH_CMD_START; assert(RUN()==FB_BATCH_INVALID_INPUT && !o.valid);
    in.command=FB_BATCH_CMD_NONE; RUN(); in.sv_valid=true; in.sv=1000;
    in.command=FB_BATCH_CMD_START; RUN(); in.elapsed_ms=100;
    RUN(); assert(o.elapsed_ms==100 && o.samples==1);
    /* Arrival dwell interrupted by invalid sample. */
    in.command=FB_BATCH_CMD_NONE; in.pv_valid=false; RUN();
    in.pv_valid=true; in.elapsed_ms=1000; RUN(); assert(!o.reached);
    RUN(); assert(o.reached);
    /* Cooling directional overshoot, zero dwell and full int32 error. */
    COMMAND(FB_BATCH_CMD_RESET); c.direction=FB_BATCH_COOLING; c.arrival_dwell_ms=0;
    in.pv=INT_MIN; in.sv=INT_MAX; in.sample_sequence=UINT_MAX;
    COMMAND(FB_BATCH_CMD_START); assert(o.overshoot==UINT64_C(4294967295));
    in.sample_sequence=0; in.pv=INT_MAX; RUN();
    assert(o.samples==2 && o.reached && isfinite(o.population_variance));
    assert(fabs(o.mean+0.5)<0.000001);
    /* Independent instance starts empty; config changes cannot mix metrics. */
    in.command=FB_BATCH_CMD_START;
    FbBatchMetrics_Execute(&other,&c,&in,&o); assert(o.samples==1 && o.elapsed_ms==0);
    in.command=FB_BATCH_CMD_NONE; ++c.arrival_band;
    assert(RUN()==FB_BATCH_INVALID_CONFIG && o.phase==FB_BATCH_FAULT && !o.valid);
    COMMAND(FB_BATCH_CMD_START); assert(o.phase==FB_BATCH_FAULT);
    COMMAND(FB_BATCH_CMD_RESET); COMMAND(FB_BATCH_CMD_START);
    s.metrics.elapsed_ms=UINT64_MAX; in.elapsed_ms=1;
    assert(RUN()==FB_BATCH_COUNTER_OVERFLOW && !o.valid);
    in.enable=false; assert(RUN()==FB_BATCH_DISABLED && !o.valid && o.samples==0);
    assert(FbBatchMetrics_Execute(NULL,&c,&in,&o)==FB_BATCH_INVALID_ARGUMENT);
    assert(FbBatchMetrics_Execute(&s,NULL,&in,&o)==FB_BATCH_INVALID_ARGUMENT);
    assert(FbBatchMetrics_Execute(&s,&c,NULL,&o)==FB_BATCH_INVALID_ARGUMENT);
    assert(FbBatchMetrics_Execute(&s,&c,&in,NULL)==FB_BATCH_INVALID_ARGUMENT);
    puts("FbBatchMetrics: sample stats, error integral, arrival, lifecycle and range PASS");
    return 0;
}
