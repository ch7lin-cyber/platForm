#include "FbRecordTrigger.h"
#include <stddef.h>
void FbRecordTrigger_Init(FbRecordTrigger *s)
{
    if (s!=NULL) *s=(FbRecordTrigger){0};
}
void FbRecordTrigger_DefaultConfig(FbRecordTriggerConfig *c)
{
    if (c!=NULL) *c=(FbRecordTriggerConfig){true,false,1000,FB_RECORD_RISING,0,0,false};
}
bool FbRecordTrigger_ValidateConfig(const FbRecordTriggerConfig *c)
{
    return c!=NULL && (!c->periodic_enabled || c->period_ms!=0) &&
        (!c->event_enabled || (c->edge>=FB_RECORD_RISING && c->edge<=FB_RECORD_BOTH));
}
static bool Same(const FbRecordTriggerConfig *a, const FbRecordTriggerConfig *b)
{
    return a->periodic_enabled==b->periodic_enabled && a->event_enabled==b->event_enabled &&
        a->period_ms==b->period_ms && a->edge==b->edge &&
        a->event_delay_ms==b->event_delay_ms && a->cooldown_ms==b->cooldown_ms &&
        a->event_one_shot==b->event_one_shot;
}
FbRecordStatus FbRecordTrigger_Execute(FbRecordTrigger *s,
    const FbRecordTriggerConfig *c, const FbRecordTriggerInput *in, FbRecordTriggerOutput *o)
{
    bool fresh, edge=false, accept=false;
    uint32_t dt;
    if (o==NULL) return FB_RECORD_INVALID_ARGUMENT;
    *o=(FbRecordTriggerOutput){0}; o->status=FB_RECORD_INVALID_ARGUMENT;
    if (s==NULL || c==NULL || in==NULL) return o->status;
    if (!in->enable || !in->run || in->reset || !FbRecordTrigger_ValidateConfig(c)) {
        FbRecordTrigger_Init(s);
        o->status=!in->enable ? FB_RECORD_DISABLED : !in->run ? FB_RECORD_STOPPED :
            in->reset ? FB_RECORD_RESET : FB_RECORD_INVALID_CONFIG;
        return o->status;
    }
    fresh=!s->configured || !Same(&s->config,c);
    if (fresh) { FbRecordTrigger_Init(s); s->config=*c; s->configured=true; }
    dt=fresh ? 0 : in->elapsed_ms;
    if (c->periodic_enabled) {
        uint64_t elapsed=(uint64_t)s->period_elapsed_ms+dt;
        uint64_t due=elapsed/c->period_ms;
        s->period_elapsed_ms=(uint32_t)(elapsed%c->period_ms);
        if (due!=0) {
            o->reasons|=FB_RECORD_REASON_PERIODIC;
            o->missed_periods=(uint32_t)(due-1);
        }
    }
    if (dt>=s->cooldown_remaining_ms) s->cooldown_remaining_ms=0;
    else s->cooldown_remaining_ms-=dt;
    if (c->event_enabled) {
        if (!in->event_valid) {
            s->event_seeded=false; s->pending=false; s->event_elapsed_ms=0;
            o->event_fault=true;
        } else {
            if (s->event_seeded && in->event!=s->previous_event)
                edge=c->edge==FB_RECORD_BOTH ||
                    (in->event ? c->edge==FB_RECORD_RISING : c->edge==FB_RECORD_FALLING);
            s->event_seeded=true; s->previous_event=in->event;
            accept=edge && !s->pending && s->cooldown_remaining_ms==0 &&
                (!c->event_one_shot || !s->event_fired);
            if (accept) { s->pending=true; s->event_elapsed_ms=0; }
            else if (s->pending) {
                uint32_t remaining=c->event_delay_ms-s->event_elapsed_ms;
                s->event_elapsed_ms+=dt>=remaining ? remaining : dt;
            }
            if (s->pending && s->event_elapsed_ms>=c->event_delay_ms) {
                o->reasons|=FB_RECORD_REASON_EVENT;
                s->pending=false; s->event_elapsed_ms=0;
                s->event_fired=true; s->cooldown_remaining_ms=c->cooldown_ms;
            }
        }
    }
    o->valid=true; o->request=o->reasons!=0;
    o->period_elapsed_ms=s->period_elapsed_ms;
    o->event_pending=s->pending; o->event_elapsed_ms=s->event_elapsed_ms;
    o->cooldown_remaining_ms=s->cooldown_remaining_ms;
    o->event_armed=c->event_enabled && s->event_seeded && !s->pending &&
        s->cooldown_remaining_ms==0 && (!c->event_one_shot || !s->event_fired);
    o->status=FB_RECORD_OK;
    return o->status;
}
