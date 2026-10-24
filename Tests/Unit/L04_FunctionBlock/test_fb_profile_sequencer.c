#include "FbProfileSequencer.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
int main(void)
{
    FbProfileStep steps[]={{100000,1000},{100000,500},{0,1000}};
    FbProfilePattern patterns[]={{steps,3,0,FB_PROFILE_END,0}};
    FbProfileConfig c={patterns,1,0,0,0,0,false,0,0};
    FbProfileInput in={0,0,true,true,0,FB_PROFILE_CMD_START};
    FbProfileOutput o;
    FbProfileSequencer s={0}, other={0};
#define RUN() FbProfileSequencer_Execute(&s,&c,&in,&o)
#define CMD(x) do {in.command=(x); RUN(); in.command=FB_PROFILE_CMD_NONE;} while(0)
    assert(FbProfileSequencer_ValidateConfig(&c));
    assert(RUN()==FB_PROFILE_OK && o.sv==0 && o.valid);
    in.command=FB_PROFILE_CMD_NONE; in.elapsed_ms=500;
    RUN(); assert(o.sv==50000 && o.step==0);
    CMD(FB_PROFILE_CMD_HOLD); assert(o.phase==FB_PROFILE_HELD && o.sv==50000);
    in.elapsed_ms=10000; RUN(); assert(o.sv==50000);
    CMD(FB_PROFILE_CMD_RESUME); assert(o.phase==FB_PROFILE_RUNNING && o.sv==50000);
    in.elapsed_ms=750; RUN(); assert(o.step==1 && o.segment_elapsed_ms==250 && o.sv==100000);
    in.elapsed_ms=750; RUN(); assert(o.step==2 && o.sv==50000);
    RUN(); assert(o.completed && o.sv==0 && !o.stop_requested);
    RUN(); assert(o.completed);
    /* START held high must not restart at completion. */
    CMD(FB_PROFILE_CMD_START); in.command=FB_PROFILE_CMD_START; in.elapsed_ms=2500;
    RUN(); assert(o.completed); RUN(); assert(o.completed);
    /* Starting step and Advance skip wait/time; STOP pulse is one call. */
    CMD(FB_PROFILE_CMD_RESET); c.start_step=1;
    CMD(FB_PROFILE_CMD_START); assert(o.step==1);
    CMD(FB_PROFILE_CMD_ADVANCE); assert(o.step==2 && o.sv==100000);
    CMD(FB_PROFILE_CMD_STOP); assert(o.phase==FB_PROFILE_STOPPED && o.stop_requested);
    RUN(); assert(!o.stop_requested);
    /* Pattern repetition then link to STOP. */
    { FbProfileStep one[]={{10,10}}, two[]={{20,10}};
      FbProfilePattern bank[]={{one,1,1,FB_PROFILE_LINK,1},{two,1,0,FB_PROFILE_STOP,0}};
      CMD(FB_PROFILE_CMD_RESET); c.patterns=bank; c.count=2; c.start_step=0;
      CMD(FB_PROFILE_CMD_START); in.elapsed_ms=15; RUN();
      assert(o.pattern==0 && o.repetition==1 && o.segment_elapsed_ms==5);
      in.elapsed_ms=15; RUN(); assert(o.phase==FB_PROFILE_STOPPED && o.sv==20 && o.stop_requested);
      /* A second runtime never inherits the first runtime. */
      in.command=FB_PROFILE_CMD_START;
      FbProfileSequencer_Execute(&other,&c,&in,&o); assert(o.sv==0 && o.segment_elapsed_ms==0);
      in.command=FB_PROFILE_CMD_NONE;
      CMD(FB_PROFILE_CMD_RESET);
    }
    c.patterns=patterns; c.count=1; c.wait_enabled=true; c.wait_band=100; c.wait_timeout_ms=500;
    CMD(FB_PROFILE_CMD_START); in.elapsed_ms=1100; in.pv=0; RUN();
    assert(o.phase==FB_PROFILE_WAITING && o.waiting_ms==100);
    CMD(FB_PROFILE_CMD_HOLD); RUN(); assert(o.phase==FB_PROFILE_HELD && o.waiting_ms==100);
    CMD(FB_PROFILE_CMD_RESUME);
    in.elapsed_ms=200; RUN(); assert(o.waiting_ms==300);
    in.pv=99900; RUN(); assert(o.step==1 && o.segment_elapsed_ms==0); /* inclusive band, no backdating */
    in.pv=0; in.elapsed_ms=1000;
    assert(RUN()==FB_PROFILE_WAIT_TIMEOUT && o.timeout_requested && o.stop_requested && !o.valid);
    assert(RUN()==FB_PROFILE_WAIT_TIMEOUT && !o.stop_requested && !o.timeout_requested);
    CMD(FB_PROFILE_CMD_START); assert(o.phase==FB_PROFILE_FAULT);
    CMD(FB_PROFILE_CMD_RESET); CMD(FB_PROFILE_CMD_START);
    in.pv_valid=false; in.elapsed_ms=1000;
    assert(RUN()==FB_PROFILE_INVALID_INPUT && !o.valid);
    /* No timeout, waiting accumulation saturates. */
    CMD(FB_PROFILE_CMD_RESET); c.wait_timeout_ms=0; in.pv_valid=true;
    CMD(FB_PROFILE_CMD_START); in.elapsed_ms=UINT32_MAX; RUN();
    assert(o.phase==FB_PROFILE_WAITING); RUN(); assert(o.waiting_ms==UINT32_MAX);
    /* Zero duration and start slope; 60000 milli-C/min -> 1 C/s. */
    CMD(FB_PROFILE_CMD_RESET); c.wait_enabled=false; steps[0].duration_ms=0;
    c.starting_rate=60000; CMD(FB_PROFILE_CMD_START); assert(o.segment_duration_ms==100000);
    in.elapsed_ms=50000; RUN(); assert(o.sv==50000);
    /* Full signed range interpolation without signed product overflow. */
    CMD(FB_PROFILE_CMD_RESET); c.starting_rate=0;
    steps[0]=(FbProfileStep){INT_MAX,UINT32_MAX}; in.initial_sv=INT_MIN;
    CMD(FB_PROFILE_CMD_START); in.elapsed_ms=UINT32_MAX-1; RUN(); assert(o.sv==INT_MAX-1);
    /* Unrepresentable start slope duration rejects safely. */
    CMD(FB_PROFILE_CMD_RESET); steps[0].duration_ms=0; c.starting_rate=1;
    CMD(FB_PROFILE_CMD_START); assert(o.status==FB_PROFILE_INVALID_CONFIG && !o.valid);
    /* Zero-time cyclic recipes terminate bounded work with an explicit fault. */
    CMD(FB_PROFILE_CMD_RESET); c.starting_rate=0; in.initial_sv=0;
    patterns[0].count=1; patterns[0].link=FB_PROFILE_LINK;
    CMD(FB_PROFILE_CMD_START); in.elapsed_ms=0;
    assert(RUN()==FB_PROFILE_TRANSITION_OVERFLOW && o.stop_requested);
    /* Recipe mutation requires reset, not silently switching a running recipe. */
    CMD(FB_PROFILE_CMD_RESET); patterns[0].link=FB_PROFILE_END; steps[0].duration_ms=1000;
    CMD(FB_PROFILE_CMD_START); ++c.revision;
    assert(RUN()==FB_PROFILE_INVALID_CONFIG);
    in.enable=false; assert(RUN()==FB_PROFILE_DISABLED && !o.valid && s.phase==FB_PROFILE_IDLE);
    in.enable=true; c.start_step=99;
    assert(RUN()==FB_PROFILE_INVALID_CONFIG);
    assert(!FbProfileSequencer_ValidateConfig(NULL));
    assert(FbProfileSequencer_Execute(NULL,&c,&in,&o)==FB_PROFILE_INVALID_ARGUMENT);
    assert(FbProfileSequencer_Execute(&s,NULL,&in,&o)==FB_PROFILE_INVALID_ARGUMENT);
    assert(FbProfileSequencer_Execute(&s,&c,NULL,&o)==FB_PROFILE_INVALID_ARGUMENT);
    assert(FbProfileSequencer_Execute(&s,&c,&in,NULL)==FB_PROFILE_INVALID_ARGUMENT);
    puts("FbProfileSequencer: ramp/soak, repeats/links, commands, wait/fault and arithmetic PASS");
    return 0;
}
