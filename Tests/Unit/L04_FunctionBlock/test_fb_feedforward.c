#include "FbFeedforward.h"
#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
int main(void)
{
    FbFeedforward s={0}, other={0};
    FbFeedforwardConfig c={100,{200,100,FB_FF_STEP,0},{200,-100,FB_FF_STEP,0},200,0};
    FbFeedforwardInput in={true,true,true,false,999};
    FbFeedforwardOutput o;
#define RUN() FbFeedforward_Execute(&s,&c,&in,&o)
#define RESTART() do {FbFeedforward_Init(&s); in.trigger=true; in.reset=false; RUN();} while(0)
    assert(FbFeedforward_ValidateConfig(&c));
    RUN(); assert(o.started && o.phase==FB_FF_DELAY && o.elapsed_ms==0 && o.value==0);
    in.elapsed_ms=100; RUN(); assert(o.phase==FB_FF_STAGE1 && o.value==100);
    in.elapsed_ms=200; RUN(); assert(o.phase==FB_FF_STAGE2 && o.value==-100);
    RUN(); assert(o.phase==FB_FF_DECAY && o.value==-100);
    in.elapsed_ms=100; RUN(); assert(o.value==-50);
    RUN(); assert(o.completed && !o.active && o.value==0);
    RUN(); assert(!o.completed && !o.started && o.phase==FB_FF_DONE);
    in.trigger=false; RUN(); in.trigger=true; RUN(); assert(o.started);
    /* Tx=0 event-low cancels, reset/high never automatically restarts. */
    in.trigger=false; RUN(); assert(o.aborted && o.value==0);
    in.reset=true; in.trigger=true; assert(RUN()==FB_FF_RESET);
    in.reset=false; RUN(); assert(!o.started && !o.active);
    in.trigger=false; RUN(); in.trigger=true; RUN(); assert(o.started);
    /* Force starts at trigger, includes delay; hard cutoff even if high. */
    c.force_ms=50; RESTART(); in.elapsed_ms=50;
    RUN(); assert(o.aborted && o.value==0 && o.phase==FB_FF_DONE);
    c.force_ms=350; RESTART(); in.trigger=false; in.elapsed_ms=300;
    RUN(); assert(o.phase==FB_FF_STAGE2 && o.value==-100 && !o.aborted);
    in.elapsed_ms=50; RUN(); assert(o.aborted && o.value==0);
    /* Forced waveform can finish before deadline, no post-wave holding. */
    c.force_ms=1000; RESTART(); in.trigger=false; in.elapsed_ms=700;
    RUN(); assert(o.completed && !o.aborted && o.value==0);
    /* Re-trigger during active is ignored (not restart/extend). */
    RESTART(); in.trigger=false; in.elapsed_ms=100; RUN();
    in.trigger=true; RUN(); assert(!o.started && o.elapsed_ms==200 && o.phase==FB_FF_STAGE1);
    /* Exponential normalized endpoint, prior actual target seeds stage2. */
    c=(FbFeedforwardConfig){0,{1000,1000,FB_FF_EXPONENTIAL,1000},
        {1000,-1000,FB_FF_EXPONENTIAL,1000},1000,3000};
    RESTART(); assert(o.phase==FB_FF_STAGE1 && o.value==0);
    in.elapsed_ms=500; RUN(); assert(o.value>=622 && o.value<=623);
    RUN(); assert(o.phase==FB_FF_STAGE2 && o.value==1000);
    RUN(); assert(o.value>=-245 && o.value<=-244);
    RUN(); assert(o.phase==FB_FF_DECAY && o.value==-1000);
    RUN(); assert(o.value==-500);
    RUN(); assert(o.completed && o.value==0);
    /* Zero-time stages skipped without applying their nonzero targets. */
    c.stage1.duration_ms=0; c.stage2.shape=FB_FF_EXPONENTIAL;
    RESTART(); assert(o.phase==FB_FF_STAGE2 && o.value==0);
    c.stage2.duration_ms=0; c.decay_ms=0;
    RESTART(); assert(o.started && o.completed && o.value==0);
    /* Large dt resolves directly without iteration or uint32 overflow. */
    c.stage1.duration_ms=1000;
    RESTART(); in.elapsed_ms=UINT_MAX; RUN(); assert(o.completed && o.value==0);
    /* Config edits cancel and require low/high edge. */
    RESTART(); ++c.stage1.target;
    RUN(); assert(o.aborted && !o.active && o.value==0);
    RUN(); assert(!o.started);
    in.trigger=false; RUN(); in.trigger=true; RUN(); assert(o.started);
    /* Invalid trigger fails to zero, consumes held trigger on recovery. */
    in.trigger_valid=false;
    assert(RUN()==FB_FF_INVALID_INPUT && !o.valid && o.value==0);
    in.trigger_valid=true; RUN(); assert(!o.started);
    /* Each instance owns its time; independent startup at zero. */
    in.elapsed_ms=100;
    FbFeedforward_Execute(&other,&c,&in,&o); assert(o.started && o.elapsed_ms==0);
    c.stage1.tau_ms=0; assert(!FbFeedforward_ValidateConfig(&c));
    assert(RUN()==FB_FF_INVALID_CONFIG);
    in.enable=false; assert(RUN()==FB_FF_DISABLED && !o.valid);
    in.enable=true; c.stage1.tau_ms=1000;
    c.stage1.target=1501; assert(!FbFeedforward_ValidateConfig(&c));
    c.stage1.target=0; c.delay_ms=3000001; assert(!FbFeedforward_ValidateConfig(&c));
    FbFeedforward_DefaultConfig(&c); assert(FbFeedforward_ValidateConfig(&c));
    assert(FbFeedforward_Execute(NULL,&c,&in,&o)==FB_FF_INVALID_ARGUMENT);
    assert(FbFeedforward_Execute(&s,NULL,&in,&o)==FB_FF_INVALID_ARGUMENT);
    assert(FbFeedforward_Execute(&s,&c,NULL,&o)==FB_FF_INVALID_ARGUMENT);
    assert(FbFeedforward_Execute(&s,&c,&in,NULL)==FB_FF_INVALID_ARGUMENT);
    assert(!FbFeedforward_ValidateConfig(NULL)); FbFeedforward_DefaultConfig(NULL);
    puts("FbFeedforward: stages, exponential, force/event, cancellation and lifecycle PASS");
    return 0;
}
