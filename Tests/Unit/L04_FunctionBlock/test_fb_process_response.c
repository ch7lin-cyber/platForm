#include "FbProcessResponse.h"
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
int main(void)
{
    FbProcessResponse s={0}, other={0};
    FbProcessResponseConfig c={1000,100,0,1,1,50,50,200,3,1000};
    FbProcessResponseInput in={0,100,0,0,1,999,true,true,true,FB_RESPONSE_CMD_START};
    FbProcessResponseOutput o;
#define RUN() FbProcessResponse_Execute(&s,&c,&in,&o)
#define RESET() do {in.command=FB_RESPONSE_CMD_RESET; RUN(); in.command=FB_RESPONSE_CMD_NONE;} while(0)
#define START() do {in.command=FB_RESPONSE_CMD_START; RUN(); in.command=FB_RESPONSE_CMD_NONE;} while(0)
    assert(FbProcessResponse_ValidateConfig(&c));
    RUN(); assert(o.phase==FB_RESPONSE_OBSERVING && o.elapsed_ms==0 && !o.result_valid);
    in.command=FB_RESPONSE_CMD_NONE; in.elapsed_ms=100;
    in.pv=100; ++in.sample_sequence; RUN();
    assert(o.onset_seen && o.onset_ms==100 && o.t10_seen && o.t10_ms==100);
    in.pv=632; ++in.sample_sequence; RUN(); assert(o.t632_seen && o.t632_ms==200);
    in.pv=900; ++in.sample_sequence; RUN(); assert(o.t90_seen && o.t90_ms==300);
    in.pv=1000; ++in.sample_sequence; RUN(); assert(!o.result_valid);
    in.pv=950; ++in.sample_sequence; RUN(); assert(!o.result_valid);
    in.pv=1050; ++in.sample_sequence; RUN();
    assert(o.result_valid && o.phase==FB_RESPONSE_COMPLETED && o.settling_ms==600);
    assert(o.final_mean==1000 && o.reference_gain==10 && o.measured_gain==10 && o.overshoot==50);
    RUN(); assert(o.elapsed_ms==600 && o.result_valid);
    /* Falling response and negative MV step have positive process gain. */
    RESET(); c.final_pv=-1000; c.step_mv=-100; in.mv=-100; in.pv=0;
    START(); in.pv=-100; ++in.sample_sequence; RUN(); assert(o.t10_seen);
    in.pv=-1000; ++in.sample_sequence; RUN();
    ++in.sample_sequence; RUN(); ++in.sample_sequence; RUN();
    assert(o.result_valid && o.measured_gain==10);
    /* Duplicate sequences do not manufacture settling samples. */
    RESET(); in.pv=-1000; START();
    RUN(); RUN(); assert(o.samples==1 && !o.result_valid);
    ++in.sample_sequence; RUN(); assert(!o.result_valid);
    ++in.sample_sequence; RUN(); assert(o.result_valid);
    /* Leaving the band restarts settling dwell and sample count. */
    RESET(); in.pv=0; START();
    in.pv=-1000; ++in.sample_sequence; RUN();
    in.pv=-1100; ++in.sample_sequence; RUN(); assert(o.overshoot==100);
    in.pv=-1000; ++in.sample_sequence; RUN();
    ++in.sample_sequence; RUN(); assert(!o.result_valid);
    ++in.sample_sequence; RUN(); assert(o.result_valid && o.settling_ms==500);
    /* Any invalid PV/MV or changed step condition invalidates experiment. */
    RESET(); in.pv=0; START(); in.valid=false;
    assert(RUN()==FB_RESPONSE_INVALID_INPUT && !o.result_valid);
    in.valid=true; START(); assert(o.phase==FB_RESPONSE_FAULT);
    RESET(); START(); in.mv=-99;
    assert(RUN()==FB_RESPONSE_EXPERIMENT_CHANGED);
    RESET(); c.mv_tolerance=1; START(); assert(o.phase==FB_RESPONSE_OBSERVING);
    ++c.onset_change; assert(RUN()==FB_RESPONSE_INVALID_CONFIG);
    RESET(); START(); in.elapsed_ms=UINT_MAX;
    assert(RUN()==FB_RESPONSE_TIMEOUT && !o.result_valid);
    /* Complete exactly at deadline is accepted, after deadline rejected. */
    RESET(); c.timeout_ms=200; c.settling_dwell_ms=200; in.pv=-1000;
    START(); in.elapsed_ms=100; ++in.sample_sequence; RUN(); ++in.sample_sequence; RUN();
    assert(o.result_valid && o.elapsed_ms==200);
    /* Cancel preserves partial diagnostics but never yields valid result. */
    RESET(); c.timeout_ms=1000; START();
    in.command=FB_RESPONSE_CMD_STOP; RUN();
    assert(o.phase==FB_RESPONSE_CANCELLED && !o.result_valid);
    /* Full int32 PV difference and sequence wrap remain defined. */
    RESET(); c.final_pv=INT_MAX; c.step_mv=100; c.settling_band=0; c.settling_dwell_ms=0;
    c.minimum_settling_samples=2; in.baseline_pv=INT_MIN; in.mv=100; in.pv=INT_MIN;
    in.sample_sequence=UINT_MAX; START();
    in.sample_sequence=0; in.pv=INT_MAX; RUN();
    ++in.sample_sequence; RUN();
    assert(o.result_valid && isfinite(o.measured_gain) && o.t90_seen);
    /* Independent runtime and invalid reference separation. */
    in.command=FB_RESPONSE_CMD_START;
    FbProcessResponse_Execute(&other,&c,&in,&o); assert(o.samples==1 && o.elapsed_ms==0);
    RESET(); in.baseline_valid=false; START(); assert(o.phase==FB_RESPONSE_FAULT);
    in.enable=false; assert(RUN()==FB_RESPONSE_DISABLED && o.samples==0);
    c.minimum_mv_step=0; assert(!FbProcessResponse_ValidateConfig(&c));
    assert(!FbProcessResponse_ValidateConfig(NULL));
    assert(FbProcessResponse_Execute(NULL,&c,&in,&o)==FB_RESPONSE_INVALID_ARGUMENT);
    assert(FbProcessResponse_Execute(&s,NULL,&in,&o)==FB_RESPONSE_INVALID_ARGUMENT);
    assert(FbProcessResponse_Execute(&s,&c,NULL,&o)==FB_RESPONSE_INVALID_ARGUMENT);
    assert(FbProcessResponse_Execute(&s,&c,&in,NULL)==FB_RESPONSE_INVALID_ARGUMENT);
    puts("FbProcessResponse: crossing times, gain, settling, quality and range PASS");
    return 0;
}
