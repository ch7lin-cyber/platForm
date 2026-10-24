#include "FbRecordTrigger.h"
#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
int main(void)
{
    FbRecordTrigger s={0}, other={0};
    FbRecordTriggerConfig c;
    FbRecordTriggerInput in={true,true,false,false,true,9999};
    FbRecordTriggerOutput o;
#define RUN() FbRecordTrigger_Execute(&s,&c,&in,&o)
    FbRecordTrigger_DefaultConfig(&c);
    assert(FbRecordTrigger_ValidateConfig(&c));
    RUN(); assert(!o.request && o.period_elapsed_ms==0);
    in.elapsed_ms=500; RUN(); assert(!o.request && o.period_elapsed_ms==500);
    RUN(); assert(o.request && o.reasons==FB_RECORD_REASON_PERIODIC && o.missed_periods==0);
    in.elapsed_ms=3500; RUN(); assert(o.request && o.missed_periods==2 && o.period_elapsed_ms==500);
    in.elapsed_ms=500; RUN(); assert(o.request && o.period_elapsed_ms==0);
    c.event_enabled=true; c.event_delay_ms=200; c.cooldown_ms=300;
    RUN(); assert(!o.request && o.event_armed); /* config change seeds */
    in.event=true; in.elapsed_ms=100; RUN();
    assert(!o.request && o.event_pending && o.event_elapsed_ms==0);
    RUN(); assert(o.event_pending && o.event_elapsed_ms==100);
    in.event=false; RUN(); assert(o.request && o.reasons==FB_RECORD_REASON_EVENT);
    assert(o.cooldown_remaining_ms==300);
    in.event=true; RUN(); assert(!o.event_pending && !o.request); /* ignored during cooldown */
    in.event=false; in.elapsed_ms=200; RUN(); assert(o.event_armed);
    in.event=true; in.elapsed_ms=100; RUN(); assert(o.event_pending);
    in.event_valid=false; RUN(); assert(o.event_fault && !o.event_pending && o.valid);
    in.event_valid=true; RUN(); assert(!o.event_pending); /* reseed high, no new edge */
    /* Both reasons coalesce into a single request. */
    c.event_delay_ms=0; c.cooldown_ms=0; c.period_ms=100;
    in.event=false; RUN();
    in.event=true; in.elapsed_ms=100; RUN();
    assert(o.request && o.reasons==(FB_RECORD_REASON_EVENT|FB_RECORD_REASON_PERIODIC));
    /* One-shot events do not stop periodic records; reset re-arms. */
    c.event_one_shot=true; in.event=false; RUN();
    in.event=true; RUN(); assert((o.reasons&FB_RECORD_REASON_EVENT) && !o.event_armed);
    in.event=false; RUN(); in.event=true; RUN();
    assert(o.reasons==FB_RECORD_REASON_PERIODIC);
    in.reset=true; assert(RUN()==FB_RECORD_RESET && !o.request);
    in.reset=false; RUN(); assert(!o.request && o.event_armed); /* high seed, no event */
    in.event=false; RUN(); in.event=true; RUN(); assert(o.reasons&FB_RECORD_REASON_EVENT);
    /* Invalid event does not invent edges or block periodic requests. */
    in.event_valid=false; RUN(); assert(o.event_fault && o.reasons==FB_RECORD_REASON_PERIODIC);
    in.event_valid=true; RUN(); assert(!(o.reasons&FB_RECORD_REASON_EVENT));
    /* Falling and both edges, initial high never triggers. */
    c.periodic_enabled=false; c.event_one_shot=false; c.edge=FB_RECORD_FALLING;
    in.event=true; RUN(); assert(!o.request);
    in.event=false; RUN(); assert(o.request);
    in.event=true; RUN(); assert(!o.request);
    c.edge=FB_RECORD_BOTH; RUN(); assert(!o.request);
    in.event=false; RUN(); assert(o.request);
    in.event=true; RUN(); assert(o.request);
    /* Largest dt and delay do not wrap, no repeated history loop. */
    c.event_delay_ms=UINT32_MAX; RUN();
    in.event=false; in.elapsed_ms=1; RUN(); assert(o.event_pending);
    in.elapsed_ms=UINT32_MAX; RUN(); assert(o.request && !o.event_pending);
    c.event_enabled=false; c.periodic_enabled=true; c.period_ms=1;
    RUN(); in.elapsed_ms=UINT32_MAX; RUN();
    assert(o.request && o.missed_periods==UINT32_MAX-1 && o.period_elapsed_ms==0);
    /* Independent runtime starts phase at zero. */
    FbRecordTrigger_Execute(&other,&c,&in,&o); assert(!o.request);
    c.period_ms=0; assert(RUN()==FB_RECORD_INVALID_CONFIG && !o.valid);
    in.run=false; assert(RUN()==FB_RECORD_STOPPED && !o.request);
    in.enable=false; assert(RUN()==FB_RECORD_DISABLED);
    in.enable=in.run=true; c.period_ms=1; RUN(); assert(!o.request);
    assert(FbRecordTrigger_Execute(NULL,&c,&in,&o)==FB_RECORD_INVALID_ARGUMENT);
    assert(FbRecordTrigger_Execute(&s,NULL,&in,&o)==FB_RECORD_INVALID_ARGUMENT);
    assert(FbRecordTrigger_Execute(&s,&c,NULL,&o)==FB_RECORD_INVALID_ARGUMENT);
    assert(FbRecordTrigger_Execute(&s,&c,&in,NULL)==FB_RECORD_INVALID_ARGUMENT);
    assert(!FbRecordTrigger_ValidateConfig(NULL)); FbRecordTrigger_DefaultConfig(NULL);
    puts("FbRecordTrigger: periodic/event, delay/cooldown, one-shot, faults and overflow PASS");
    return 0;
}
