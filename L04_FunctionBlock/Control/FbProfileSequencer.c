#include "FbProfileSequencer.h"

void FbProfileSequencer_Init(FbProfileSequencer *s)
{
    if (s!=NULL) *s=(FbProfileSequencer){0};
}
bool FbProfileSequencer_ValidateConfig(const FbProfileConfig *c)
{
    size_t i;
    if (c==NULL || c->patterns==NULL || c->count==0 ||
        c->count>FB_PROFILE_MAX_PATTERNS || c->start_pattern>=c->count) return false;
    for (i=0;i<c->count;++i) {
        const FbProfilePattern *p=&c->patterns[i];
        if (p->steps==NULL || p->count==0 || p->count>FB_PROFILE_MAX_STEPS ||
            p->link<FB_PROFILE_LINK || p->link>FB_PROFILE_STOP ||
            (p->link==FB_PROFILE_LINK && p->next_pattern>=c->count)) return false;
    }
    return c->start_step<c->patterns[c->start_pattern].count;
}
static bool Same(const FbProfileConfig *a, const FbProfileConfig *b)
{
    return a->patterns==b->patterns && a->count==b->count && a->revision==b->revision &&
        a->start_pattern==b->start_pattern && a->start_step==b->start_step &&
        a->starting_rate==b->starting_rate && a->wait_enabled==b->wait_enabled &&
        a->wait_band==b->wait_band && a->wait_timeout_ms==b->wait_timeout_ms;
}
static void Fault(FbProfileSequencer *s, FbProfileOutput *o, FbProfileStatus status)
{
    s->phase=FB_PROFILE_FAULT; s->fault_status=status;
    o->stop_requested=true;
    o->timeout_requested=status==FB_PROFILE_WAIT_TIMEOUT;
}
static void Enter(FbProfileSequencer *s)
{
    s->segment_start=s->sv; s->elapsed_ms=0; s->waiting_ms=0;
    s->duration_ms=s->config.patterns[s->pattern].steps[s->step].duration_ms;
    s->phase=FB_PROFILE_RUNNING;
}
static void Next(FbProfileSequencer *s, FbProfileOutput *o)
{
    const FbProfilePattern *p=&s->config.patterns[s->pattern];
    if (++s->step<p->count) { Enter(s); return; }
    if (s->repetition<p->repeat_count) {
        ++s->repetition; s->step=0; Enter(s); return;
    }
    if (p->link==FB_PROFILE_LINK) {
        s->pattern=p->next_pattern; s->step=0; s->repetition=0; Enter(s);
    } else {
        --s->step;
        s->phase=p->link==FB_PROFILE_END ? FB_PROFILE_COMPLETED : FB_PROFILE_STOPPED;
        o->stop_requested=p->link==FB_PROFILE_STOP;
    }
}
static FbProfileStatus Publish(FbProfileSequencer *s, FbProfileOutput *o)
{
    o->phase=s->phase;
    o->status=s->phase==FB_PROFILE_FAULT ? s->fault_status : FB_PROFILE_OK;
    o->valid=s->started && s->phase!=FB_PROFILE_FAULT;
    if (o->valid) {
        o->sv=s->sv; o->pattern=s->pattern; o->step=s->step;
        o->target=s->config.patterns[s->pattern].steps[s->step].target;
        o->repetition=s->repetition; o->segment_elapsed_ms=s->elapsed_ms;
        o->segment_duration_ms=s->duration_ms; o->waiting_ms=s->waiting_ms;
        o->completed=s->phase==FB_PROFILE_COMPLETED;
    }
    return o->status;
}
FbProfileStatus FbProfileSequencer_Execute(FbProfileSequencer *s,
    const FbProfileConfig *c, const FbProfileInput *in, FbProfileOutput *o)
{
    bool command, was_waiting;
    uint32_t remaining;
    unsigned transitions=0;
    if (o==NULL) return FB_PROFILE_INVALID_ARGUMENT;
    *o=(FbProfileOutput){0}; o->status=FB_PROFILE_INVALID_ARGUMENT;
    if (s==NULL || c==NULL || in==NULL) return o->status;
    if (!in->enable) {
        FbProfileSequencer_Init(s); o->status=FB_PROFILE_DISABLED; return o->status;
    }
    command=in->command!=FB_PROFILE_CMD_NONE && in->command!=s->previous_command;
    s->previous_command=in->command;
    if (command && in->command==FB_PROFILE_CMD_RESET) {
        FbProfileSequencer_Init(s); s->previous_command=in->command; return Publish(s,o);
    }
    if (s->phase==FB_PROFILE_FAULT) return Publish(s,o);
    if (in->command<FB_PROFILE_CMD_NONE || in->command>FB_PROFILE_CMD_RESET) {
        Fault(s,o,FB_PROFILE_INVALID_INPUT); return Publish(s,o);
    }
    if (!FbProfileSequencer_ValidateConfig(c) ||
        (s->configured && !Same(&s->config,c))) {
        Fault(s,o,FB_PROFILE_INVALID_CONFIG); return Publish(s,o);
    }
    if (command && in->command==FB_PROFILE_CMD_STOP) {
        s->phase=FB_PROFILE_STOPPED; o->stop_requested=true; return Publish(s,o);
    }
    if (command && in->command==FB_PROFILE_CMD_START &&
        (s->phase==FB_PROFILE_IDLE || s->phase==FB_PROFILE_COMPLETED || s->phase==FB_PROFILE_STOPPED)) {
        uint64_t duration;
        int64_t delta;
        s->config=*c; s->configured=true; s->started=true;
        s->pattern=c->start_pattern; s->step=c->start_step; s->repetition=0;
        s->sv=in->initial_sv; Enter(s);
        if (s->duration_ms==0 && c->starting_rate!=0) {
            delta=(int64_t)c->patterns[s->pattern].steps[s->step].target-s->sv;
            if (delta<0) delta=-delta;
            duration=((uint64_t)delta*60000+c->starting_rate-1)/c->starting_rate;
            if (duration>UINT32_MAX) Fault(s,o,FB_PROFILE_INVALID_CONFIG);
            else s->duration_ms=(uint32_t)duration;
        }
        return Publish(s,o);
    }
    if (command && in->command==FB_PROFILE_CMD_HOLD &&
        (s->phase==FB_PROFILE_RUNNING || s->phase==FB_PROFILE_WAITING)) {
        s->held_phase=s->phase; s->phase=FB_PROFILE_HELD; return Publish(s,o);
    }
    if (command && in->command==FB_PROFILE_CMD_RESUME && s->phase==FB_PROFILE_HELD) {
        s->phase=s->held_phase; return Publish(s,o);
    }
    if (command && in->command==FB_PROFILE_CMD_ADVANCE &&
        (s->phase==FB_PROFILE_RUNNING || s->phase==FB_PROFILE_WAITING)) {
        s->sv=c->patterns[s->pattern].steps[s->step].target; Next(s,o); return Publish(s,o);
    }
    remaining=in->elapsed_ms;
    was_waiting=s->phase==FB_PROFILE_WAITING;
    while (s->phase==FB_PROFILE_RUNNING || s->phase==FB_PROFILE_WAITING) {
        int32_t target=c->patterns[s->pattern].steps[s->step].target;
        if (s->phase==FB_PROFILE_RUNNING) {
            uint32_t needed=s->duration_ms-s->elapsed_ms;
            uint32_t take=remaining<needed ? remaining : needed;
            s->elapsed_ms+=take; remaining-=take;
            if (s->duration_ms!=0) {
                int64_t delta=(int64_t)target-s->segment_start;
                uint64_t magnitude=(uint64_t)(delta<0 ? -delta : delta);
                int64_t moved=(int64_t)(magnitude*s->elapsed_ms/s->duration_ms);
                s->sv=(int32_t)((int64_t)s->segment_start+(delta<0 ? -moved : moved));
            }
            if (s->elapsed_ms<s->duration_ms) break;
            s->sv=target; s->phase=FB_PROFILE_WAITING;
        }
        if (c->wait_enabled) {
            int64_t difference=(int64_t)in->pv-target;
            if (!in->pv_valid) { Fault(s,o,FB_PROFILE_INVALID_INPUT); break; }
            if (difference<0) difference=-difference;
            if ((uint64_t)difference>c->wait_band) {
                uint32_t available=UINT32_MAX-s->waiting_ms;
                s->waiting_ms+=remaining>available ? available : remaining;
                if (c->wait_timeout_ms!=0 && s->waiting_ms>=c->wait_timeout_ms)
                    Fault(s,o,FB_PROFILE_WAIT_TIMEOUT);
                break;
            }
            /* Arrival is observed now, not retroactively during the last dt. */
            if (was_waiting) remaining=0;
        }
        if (++transitions>FB_PROFILE_TRANSITION_LIMIT) {
            Fault(s,o,FB_PROFILE_TRANSITION_OVERFLOW); break;
        }
        Next(s,o);
        was_waiting=false;
    }
    return Publish(s,o);
}
