#include "FbInBandTimer.h"
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>

static void ok(const FbInBandTimerConfig *c, const FbInBandTimerInput *i,
               FbInBandTimerState *s, FbInBandTimerOutput *o,
               float elapsed, bool band, bool done)
{
    assert(FbInBandTimer_Execute(c, i, s, o) == FB_IN_BAND_TIMER_OK);
    assert(o->status == FB_IN_BAND_TIMER_OK && o->valid);
    assert(o->elapsed_seconds == elapsed && o->in_band == band && o->done == done);
}
static void held(const FbInBandTimerConfig *c, const FbInBandTimerInput *i,
                 FbInBandTimerState *s, FbInBandTimerOutput *o,
                 FbInBandTimerStatus expected)
{
    double elapsed = s->elapsed_seconds, correction = s->compensation;
    bool initialized = s->initialized;
    assert(FbInBandTimer_Execute(c, i, s, o) == expected);
    assert(o->status == expected && !o->valid && !o->in_band && !o->done);
    assert(s->elapsed_seconds == elapsed && s->compensation == correction &&
           s->initialized == initialized);
}
int main(void)
{
    FbInBandTimerConfig c = {2.0f, 1.0f, FB_IN_BAND_TIMER_PAUSE};
    FbInBandTimerInput i = {100.0f, 100.0f, 0.25f, true, true};
    FbInBandTimerState s = {0}, other = {0};
    FbInBandTimerOutput o;
    const float bad[] = {NAN, INFINITY, -INFINITY};
    size_t k;
    /* UT-01: uninitialized state rejected, Init creates zero history. */
    held(&c,&i,&s,&o,FB_IN_BAND_TIMER_INVALID_STATE);
    assert(FbInBandTimer_Init(&s) == FB_IN_BAND_TIMER_OK);
    assert(s.initialized && s.elapsed_seconds == 0.0 && s.compensation == 0.0);
    /* UT-02: fractional dt and exact completion, saturation no wrap. */
    ok(&c,&i,&s,&o,0.25f,true,false);
    ok(&c,&i,&s,&o,0.5f,true,false);
    ok(&c,&i,&s,&o,0.75f,true,false);
    ok(&c,&i,&s,&o,1.0f,true,true);
    ok(&c,&i,&s,&o,1.0f,true,true);
    /* UT-03: both inclusive boundaries; outside PAUSE clears done not time. */
    i.pv=98.0f; ok(&c,&i,&s,&o,1.0f,true,true);
    i.pv=102.0f; ok(&c,&i,&s,&o,1.0f,true,true);
    i.pv=nextafterf(102.0f,INFINITY); ok(&c,&i,&s,&o,1.0f,false,false);
    i.pv=nextafterf(98.0f,-INFINITY); ok(&c,&i,&s,&o,1.0f,false,false);
    i.pv=100.0f; i.dt_seconds=0; ok(&c,&i,&s,&o,1.0f,true,true);
    /* UT-04: RESET out of band even zero dt clears history and completion. */
    c.out_of_band_policy=FB_IN_BAND_TIMER_RESET; i.pv=103;
    ok(&c,&i,&s,&o,0,false,false); assert(s.compensation==0);
    i.pv=100; i.dt_seconds=0.25f; ok(&c,&i,&s,&o,0.25f,true,false);
    /* UT-05: explicit Reset clears accumulated time without advancing. */
    assert(FbInBandTimer_Reset(&s)==FB_IN_BAND_TIMER_OK);
    assert(s.elapsed_seconds==0 && s.compensation==0);
    /* UT-06: zero dt holds, then only current interval counts. */
    i.dt_seconds=0; ok(&c,&i,&s,&o,0,true,false);
    i.dt_seconds=0.25f; ok(&c,&i,&s,&o,0.25f,true,false);
    /* UT-07: partial PAUSE resumes without counting outside duration. */
    c.out_of_band_policy=FB_IN_BAND_TIMER_PAUSE; i.pv=103; i.dt_seconds=100;
    ok(&c,&i,&s,&o,0.25f,false,false);
    i.pv=100; i.dt_seconds=0.25f; ok(&c,&i,&s,&o,0.5f,true,false);
    /* UT-08: disabled ignores bad config, dt, PV, SP and quality. */
    i.enable=false; i.input_valid=false; i.pv=NAN; i.sp=INFINITY; i.dt_seconds=-INFINITY;
    c.tolerance=NAN; c.dwell_seconds=-1; c.out_of_band_policy=(FbInBandTimerOutOfBandPolicy)99;
    held(&c,&i,&s,&o,FB_IN_BAND_TIMER_DISABLED);
    assert(o.elapsed_seconds==0.5f);
    c.tolerance=2; c.dwell_seconds=1; c.out_of_band_policy=FB_IN_BAND_TIMER_PAUSE;
    i.enable=true; i.input_valid=true; i.pv=i.sp=100; i.dt_seconds=0.25f;
    ok(&c,&i,&s,&o,0.75f,true,false);
    /* UT-09: every numeric input rejects NaN, +inf, -inf atomically. */
    for(k=0;k<3;++k) {
        i.pv=bad[k]; held(&c,&i,&s,&o,FB_IN_BAND_TIMER_INVALID_INPUT); i.pv=100;
        i.sp=bad[k]; held(&c,&i,&s,&o,FB_IN_BAND_TIMER_INVALID_INPUT); i.sp=100;
        i.dt_seconds=bad[k]; held(&c,&i,&s,&o,FB_IN_BAND_TIMER_INVALID_INPUT); i.dt_seconds=0.25f;
    }
    /* UT-10: negative dt and invalid quality hold even outside RESET band. */
    i.dt_seconds=-1; held(&c,&i,&s,&o,FB_IN_BAND_TIMER_INVALID_INPUT); i.dt_seconds=0.25f;
    i.input_valid=false; i.pv=103; c.out_of_band_policy=FB_IN_BAND_TIMER_RESET;
    held(&c,&i,&s,&o,FB_IN_BAND_TIMER_INVALID_INPUT);
    i.input_valid=true; i.pv=100; c.out_of_band_policy=FB_IN_BAND_TIMER_PAUSE;
    /* UT-11: both config numerics reject all nonfinite values. */
    for(k=0;k<3;++k) {
        c.tolerance=bad[k]; held(&c,&i,&s,&o,FB_IN_BAND_TIMER_INVALID_CONFIGURATION); c.tolerance=2;
        c.dwell_seconds=bad[k]; held(&c,&i,&s,&o,FB_IN_BAND_TIMER_INVALID_CONFIGURATION); c.dwell_seconds=1;
    }
    /* UT-12: negative config and unknown policy reject atomically. */
    c.tolerance=-1; held(&c,&i,&s,&o,FB_IN_BAND_TIMER_INVALID_CONFIGURATION); c.tolerance=2;
    c.dwell_seconds=-1; held(&c,&i,&s,&o,FB_IN_BAND_TIMER_INVALID_CONFIGURATION); c.dwell_seconds=1;
    c.out_of_band_policy=(FbInBandTimerOutOfBandPolicy)-1;
    held(&c,&i,&s,&o,FB_IN_BAND_TIMER_INVALID_CONFIGURATION); c.out_of_band_policy=FB_IN_BAND_TIMER_PAUSE;
    /* UT-13: all null pointers, including disabled, no mutation. */
    held(NULL,&i,&s,&o,FB_IN_BAND_TIMER_INVALID_ARGUMENT);
    held(&c,NULL,&s,&o,FB_IN_BAND_TIMER_INVALID_ARGUMENT);
    assert(FbInBandTimer_Execute(&c,&i,NULL,&o)==FB_IN_BAND_TIMER_INVALID_ARGUMENT);
    assert(o.elapsed_seconds==0 && !o.valid);
    assert(FbInBandTimer_Execute(&c,&i,&s,NULL)==FB_IN_BAND_TIMER_INVALID_ARGUMENT);
    assert(s.elapsed_seconds==0.75);
    i.enable=false; held(NULL,&i,&s,&o,FB_IN_BAND_TIMER_INVALID_ARGUMENT); i.enable=true;
    assert(FbInBandTimer_Init(NULL)==FB_IN_BAND_TIMER_INVALID_ARGUMENT);
    assert(FbInBandTimer_Reset(NULL)==FB_IN_BAND_TIMER_INVALID_ARGUMENT);
    /* UT-14: zero tolerance equality only; zero dwell requires in band. */
    c.tolerance=0; c.dwell_seconds=0; i.dt_seconds=0;
    ok(&c,&i,&s,&o,0,true,true);
    i.pv=nextafterf(100,INFINITY); ok(&c,&i,&s,&o,0,false,false);
    /* UT-15: full float PV-SP span does not overflow band comparison. */
    c.tolerance=FLT_MAX; c.dwell_seconds=1; i.pv=FLT_MAX; i.sp=-FLT_MAX;
    ok(&c,&i,&s,&o,0,false,false);
    i.sp=0; ok(&c,&i,&s,&o,0,true,false);
    /* UT-16: huge dt saturates finite target; repeated huge dt stays finite. */
    i.dt_seconds=FLT_MAX; ok(&c,&i,&s,&o,1,true,true);
    ok(&c,&i,&s,&o,1,true,true); assert(s.compensation==0);
    /* UT-17: full float dwell supported. */
    c.dwell_seconds=FLT_MAX; ok(&c,&i,&s,&o,FLT_MAX,true,true);
    /* UT-18: lowered dwell caps even outside PAUSE at zero dt. */
    c.dwell_seconds=0.5f; i.dt_seconds=0; i.sp=-FLT_MAX;
    ok(&c,&i,&s,&o,0.5f,false,false); assert(s.compensation==0);
    /* UT-19: raised dwell resumes, SP change does not reset. */
    c.dwell_seconds=1; i.pv=i.sp=7; i.dt_seconds=0.25f;
    ok(&c,&i,&s,&o,0.75f,true,false);
    /* UT-20: instance independence, reset one does not affect other. */
    assert(FbInBandTimer_Init(&other)==FB_IN_BAND_TIMER_OK);
    ok(&c,&i,&other,&o,0.25f,true,false); assert(s.elapsed_seconds==0.75);
    assert(FbInBandTimer_Reset(&other)==FB_IN_BAND_TIMER_OK); assert(s.elapsed_seconds==0.75);
    /* UT-21: accumulation smaller than float ULP keeps progressing. */
    assert(FbInBandTimer_Reset(&s)==FB_IN_BAND_TIMER_OK);
    c.dwell_seconds=40000; i.dt_seconds=32000; ok(&c,&i,&s,&o,32000,true,false);
    i.dt_seconds=0.00001f;
    for(k=0;k<100000;++k) assert(FbInBandTimer_Execute(&c,&i,&s,&o)==FB_IN_BAND_TIMER_OK);
    assert(o.elapsed_seconds==32001 && s.elapsed_seconds>32000.99);
    /* UT-22: increments below double ULP are retained via compensation. */
    assert(FbInBandTimer_Reset(&s)==FB_IN_BAND_TIMER_OK);
    c.dwell_seconds=2.0e20f; i.dt_seconds=1.0e20f;
    assert(FbInBandTimer_Execute(&c,&i,&s,&o)==FB_IN_BAND_TIMER_OK);
    { double start=s.elapsed_seconds; i.dt_seconds=1;
      for(k=0;k<100000;++k) assert(FbInBandTimer_Execute(&c,&i,&s,&o)==FB_IN_BAND_TIMER_OK);
      assert(s.elapsed_seconds>start && (s.elapsed_seconds-start)-s.compensation==100000.0);
    }
    /* UT-23: disabled/zero dt preserve residual; saturation/reset clear it. */
    i.dt_seconds=1; assert(FbInBandTimer_Execute(&c,&i,&s,&o)==FB_IN_BAND_TIMER_OK);
    assert(s.compensation!=0);
    i.enable=false; held(&c,&i,&s,&o,FB_IN_BAND_TIMER_DISABLED); i.enable=true;
    { double a=s.elapsed_seconds,b=s.compensation; i.dt_seconds=0;
      assert(FbInBandTimer_Execute(&c,&i,&s,&o)==FB_IN_BAND_TIMER_OK);
      assert(s.elapsed_seconds==a && s.compensation==b); }
    c.dwell_seconds=1; ok(&c,&i,&s,&o,1,true,true); assert(s.compensation==0);
    assert(FbInBandTimer_Reset(&s)==FB_IN_BAND_TIMER_OK); assert(s.compensation==0);
    /* UT-24: damaged finite state and correction are rejected. */
    s.elapsed_seconds=-1; held(&c,&i,&s,&o,FB_IN_BAND_TIMER_INVALID_STATE);
    s.elapsed_seconds=(double)FLT_MAX*2; held(&c,&i,&s,&o,FB_IN_BAND_TIMER_INVALID_STATE);
    s.elapsed_seconds=1; s.compensation=1; held(&c,&i,&s,&o,FB_IN_BAND_TIMER_INVALID_STATE);
    /* UT-25: all nonfinite state fields rejected and reset recovers. */
    for(k=0;k<3;++k) {
        s.elapsed_seconds=bad[k]; s.compensation=0;
        assert(FbInBandTimer_Execute(&c,&i,&s,&o)==FB_IN_BAND_TIMER_INVALID_STATE);
        assert(!o.valid && !o.done && o.elapsed_seconds==0);
        s.elapsed_seconds=0; s.compensation=bad[k];
        assert(FbInBandTimer_Execute(&c,&i,&s,&o)==FB_IN_BAND_TIMER_INVALID_STATE);
        assert(!o.valid && o.elapsed_seconds==0);
    }
    assert(FbInBandTimer_Reset(&s)==FB_IN_BAND_TIMER_OK);
    ok(&c,&i,&s,&o,0,true,false);
    /* UT-26: rounded elapsed at target must not finish with positive residual. */
    c.dwell_seconds=1.0e20f; s.elapsed_seconds=c.dwell_seconds; s.compensation=1.0;
    i.dt_seconds=0; ok(&c,&i,&s,&o,c.dwell_seconds,true,false);
    i.dt_seconds=0.5f; ok(&c,&i,&s,&o,c.dwell_seconds,true,false);
    ok(&c,&i,&s,&o,c.dwell_seconds,true,true);
    assert(s.compensation==0);
    printf("FbInBandTimer host tests passed (26 cases); sizes: config=%lu input=%lu state=%lu output=%lu\n",
      (unsigned long)sizeof(c),(unsigned long)sizeof(i),(unsigned long)sizeof(s),(unsigned long)sizeof(o));
    return 0;
}
