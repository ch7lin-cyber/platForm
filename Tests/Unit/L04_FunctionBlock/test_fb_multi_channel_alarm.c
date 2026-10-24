#include "FbMultiChannelAlarm.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>

int main(void)
{
    bool selected[]={true,true,false};
    FbMultiChannelAlarmConfig c={selected,3,0,1000,100,false};
    FbMultiAlarmSample pv[]={{100000,true},{102000,true},{INT_MIN,false}};
    FbMultiChannelAlarmInput in={pv,3,50,true,false};
    FbMultiChannelAlarm a={0}, b={0};
    FbMultiChannelAlarmOutput o;
#define RUN() FbMultiChannelAlarm_Execute(&a,&c,&in,&o)
    assert(RUN()==FB_MULTI_ALARM_OK);
    assert(o.selected_count==2 && o.minimum==100000 && o.maximum==102000);
    assert(o.spread==2000 && o.condition_active && !o.qualified_active && o.elapsed_ms==0);
    RUN(); assert(o.elapsed_ms==50 && !o.action_request);
    RUN(); assert(o.qualified_active && o.active && o.action_request && !o.latched);
    RUN(); assert(o.qualified_active && !o.action_request);
    /* Invalid episode cannot reissue a command after recovery. */
    pv[0].valid=false;
    assert(RUN()==FB_MULTI_ALARM_INVALID_INPUT && !o.valid && !o.active && !o.action_request);
    pv[0].valid=true;
    RUN(); RUN(); RUN(); assert(o.qualified_active && !o.action_request);
    /* Exact threshold is normal, rearms non-hold. */
    pv[1].value=101000;
    RUN(); assert(!o.condition_active && !o.active && o.spread==1000);
    pv[1].value=102000;
    RUN(); RUN(); RUN(); assert(o.action_request);
    /* Pending interruption cancels timing, first recovered true starts at 0. */
    FbMultiChannelAlarm_Init(&a);
    RUN(); RUN(); pv[1].valid=false; RUN(); pv[1].valid=true;
    RUN(); assert(o.elapsed_ms==0); RUN(); assert(!o.action_request);
    RUN(); assert(o.action_request);
    /* Hold survives normal condition and invalid samples. */
    c.hold=true; c.delay_ms=0;
    RUN(); assert(o.action_request && o.latched);
    pv[1].value=100000;
    RUN(); assert(o.active && o.latched && !o.condition_active && !o.action_request);
    pv[1].valid=false;
    RUN(); assert(!o.valid && o.latched && o.active && !o.action_request);
    in.reset=true;
    RUN(); assert(o.reset_rejected && o.latched);
    pv[1].valid=true;
    RUN(); assert(!o.reset_accepted && o.latched); /* held reset does not retry */
    in.reset=false; RUN(); in.reset=true;
    RUN(); assert(o.reset_accepted && !o.latched && !o.active);
    pv[1].value=102000;
    RUN(); assert(o.action_request && o.latched); /* true, but no reset edge */
    in.reset=false; RUN(); in.reset=true;
    RUN(); assert(o.reset_rejected && o.active && !o.action_request);
    /* Changed revision creates a new episode; unselected invalid ignored. */
    in.reset=false; ++c.revision;
    RUN(); assert(o.action_request);
    /* Oversized elapsed saturates; independent instance starts unqualified. */
    c.delay_ms=10000;
    RUN(); assert(!o.action_request && o.elapsed_ms==0);
    in.elapsed_ms=UINT_MAX;
    RUN(); assert(o.action_request && o.elapsed_ms==10000);
    FbMultiChannelAlarm_Execute(&b,&c,&in,&o);
    assert(o.elapsed_ms==0 && !o.action_request);
    /* Disable clears latch and ignores invalid numeric configuration. */
    c.delay_ms=10001;
    assert(RUN()==FB_MULTI_ALARM_INVALID_CONFIG && o.latched);
    in.enable=false;
    assert(RUN()==FB_MULTI_ALARM_DISABLED && !o.latched && !o.active && !a.configured);
    in.enable=true;
    assert(RUN()==FB_MULTI_ALARM_INVALID_CONFIG);
    c.delay_ms=0; selected[1]=false;
    assert(RUN()==FB_MULTI_ALARM_INVALID_CONFIG && o.selected_count==1);
    selected[1]=true; ++c.revision;
    in.count=2;
    assert(RUN()==FB_MULTI_ALARM_INVALID_INPUT);
    in.count=3; in.samples=NULL;
    assert(RUN()==FB_MULTI_ALARM_INVALID_INPUT);
    in.samples=pv;
    /* Full signed range difference is 4294967295, with strict equality. */
    pv[0].value=INT_MIN; pv[1].value=INT_MAX;
    c.high_limit=UINT32_MAX;
    RUN(); assert(o.spread==UINT64_C(4294967295) && !o.condition_active);
    c.high_limit=UINT32_MAX-1;
    RUN(); assert(o.action_request && o.condition_active);
    assert(FbMultiChannelAlarm_Execute(NULL,&c,&in,&o)==FB_MULTI_ALARM_INVALID_ARGUMENT);
    assert(FbMultiChannelAlarm_Execute(&a,NULL,&in,&o)==FB_MULTI_ALARM_INVALID_ARGUMENT);
    assert(FbMultiChannelAlarm_Execute(&a,&c,NULL,&o)==FB_MULTI_ALARM_INVALID_ARGUMENT);
    assert(FbMultiChannelAlarm_Execute(&a,&c,&in,NULL)==FB_MULTI_ALARM_INVALID_ARGUMENT);
    puts("FbMultiChannelAlarm: extrema, timing, one-shot, latch/reset, quality and overflow PASS");
    return 0;
}
