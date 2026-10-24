#include "FbFeedforward.h"
#include <stddef.h>
#include <math.h>

void FbFeedforward_Init(FbFeedforward *s)
{
    if (s!=NULL) *s=(FbFeedforward){0};
}
void FbFeedforward_DefaultConfig(FbFeedforwardConfig *c)
{
    if (c!=NULL) *c=(FbFeedforwardConfig){0};
}
static bool StageValid(const FbFeedforwardStage *p)
{
    return p->duration_ms<=3000000 && p->target>=-1500 && p->target<=1500 &&
        (p->shape==FB_FF_STEP || p->shape==FB_FF_EXPONENTIAL) &&
        (p->shape!=FB_FF_EXPONENTIAL || p->duration_ms==0 ||
         (p->tau_ms>0 && p->tau_ms<=3000000));
}
bool FbFeedforward_ValidateConfig(const FbFeedforwardConfig *c)
{
    return c!=NULL && c->delay_ms<=3000000 && c->decay_ms<=3000000 &&
        c->force_ms<=3200000 && StageValid(&c->stage1) && StageValid(&c->stage2);
}
static bool StageSame(const FbFeedforwardStage *a, const FbFeedforwardStage *b)
{
    return a->duration_ms==b->duration_ms && a->target==b->target &&
        a->shape==b->shape && a->tau_ms==b->tau_ms;
}
static bool Same(const FbFeedforwardConfig *a, const FbFeedforwardConfig *b)
{
    return a->delay_ms==b->delay_ms && a->decay_ms==b->decay_ms &&
        a->force_ms==b->force_ms && StageSame(&a->stage1,&b->stage1) &&
        StageSame(&a->stage2,&b->stage2);
}
static bool Active(FbFeedforwardPhase p)
{
    return p>=FB_FF_DELAY && p<=FB_FF_DECAY;
}
static double Value(const FbFeedforwardStage *p, double start, uint64_t t)
{
    if (p->shape==FB_FF_STEP) return p->target;
    /* Normalized exponential reaches the target at the finite stage end.
     * expm1 avoids cancellation for duration/tau close to zero. */
    return start+((double)p->target-start)*
        (expm1(-(double)t/p->tau_ms)/expm1(-(double)p->duration_ms/p->tau_ms));
}
static void Evaluate(FbFeedforward *s, FbFeedforwardOutput *o)
{
    const FbFeedforwardConfig *c=&s->config;
    uint64_t t=s->elapsed_ms;
    uint64_t total=(uint64_t)c->delay_ms+c->stage1.duration_ms+c->stage2.duration_ms+c->decay_ms;
    double value=0;
    if (t>=total || (c->force_ms!=0 && t>=c->force_ms)) {
        s->phase=FB_FF_DONE;
        o->aborted=c->force_ms!=0 && c->force_ms<total && t>=c->force_ms;
        o->completed=!o->aborted;
        return;
    }
    if (t<c->delay_ms) { s->phase=FB_FF_DELAY; return; }
    t-=c->delay_ms;
    if (t<c->stage1.duration_ms) {
        s->phase=FB_FF_STAGE1; value=Value(&c->stage1,0,t);
    } else {
        t-=c->stage1.duration_ms;
        if (c->stage1.duration_ms!=0) value=c->stage1.target;
        if (t<c->stage2.duration_ms) {
            s->phase=FB_FF_STAGE2; value=Value(&c->stage2,value,t);
        } else {
            t-=c->stage2.duration_ms;
            if (c->stage2.duration_ms!=0) value=c->stage2.target;
            s->phase=FB_FF_DECAY;
            value*=1.0-(double)t/c->decay_ms;
        }
    }
    /* Bound numerical noise before converting; both target and start bounded. */
    if (value>1500) value=1500;
    if (value<-1500) value=-1500;
    o->value=(int32_t)value;
}
FbFeedforwardStatus FbFeedforward_Execute(FbFeedforward *s,
    const FbFeedforwardConfig *c, const FbFeedforwardInput *in, FbFeedforwardOutput *o)
{
    bool edge, active;
    if (o==NULL) return FB_FF_INVALID_ARGUMENT;
    *o=(FbFeedforwardOutput){0}; o->status=FB_FF_INVALID_ARGUMENT;
    if (s==NULL || c==NULL || in==NULL) return o->status;
    active=Active(s->phase);
    if (!in->enable || in->reset || !FbFeedforward_ValidateConfig(c) || !in->trigger_valid) {
        FbFeedforward_Init(s);
        /* Disable permits high level on later re-enable; other reset paths
         * consume current level and require a low/high edge to run again. */
        s->previous_trigger=in->enable && in->trigger;
        o->aborted=active;
        o->status=!in->enable ? FB_FF_DISABLED : in->reset ? FB_FF_RESET :
            !FbFeedforward_ValidateConfig(c) ? FB_FF_INVALID_CONFIG : FB_FF_INVALID_INPUT;
        return o->status;
    }
    o->status=FB_FF_OK; o->valid=true;
    if (s->configured && !Same(&s->config,c)) {
        FbFeedforward_Init(s); s->config=*c; s->configured=true;
        s->previous_trigger=in->trigger; o->aborted=active;
        return o->status;
    }
    if (!s->configured) { s->config=*c; s->configured=true; }
    edge=in->trigger && !s->previous_trigger;
    s->previous_trigger=in->trigger;
    if (edge && !active) {
        s->elapsed_ms=0; s->phase=FB_FF_DELAY; o->started=true;
        Evaluate(s,o);
    } else if (active) {
        if (c->force_ms==0 && !in->trigger) { s->phase=FB_FF_DONE; o->aborted=true; }
        else { s->elapsed_ms+=in->elapsed_ms; Evaluate(s,o); }
    }
    o->elapsed_ms=s->elapsed_ms; o->phase=s->phase; o->active=Active(s->phase);
    return o->status;
}
