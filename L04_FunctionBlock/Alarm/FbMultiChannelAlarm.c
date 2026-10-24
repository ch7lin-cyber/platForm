#include "FbMultiChannelAlarm.h"

void FbMultiChannelAlarm_Init(FbMultiChannelAlarm *instance)
{
    if (instance != NULL) *instance = (FbMultiChannelAlarm){0};
}
static bool Same(const FbMultiChannelAlarmConfig *a,
                 const FbMultiChannelAlarmConfig *b)
{
    return a->selected==b->selected && a->count==b->count &&
        a->revision==b->revision && a->high_limit==b->high_limit &&
        a->delay_ms==b->delay_ms && a->hold==b->hold;
}
static FbMultiAlarmStatus Invalid(FbMultiChannelAlarm *s,
    FbMultiChannelAlarmOutput *o, FbMultiAlarmStatus status, bool reset_edge)
{
    s->pending=false;
    s->elapsed_ms=0;
    o->latched=s->latched;
    o->active=s->latched;
    o->reset_rejected=reset_edge;
    o->status=status;
    return status;
}
FbMultiAlarmStatus FbMultiChannelAlarm_Execute(FbMultiChannelAlarm *s,
    const FbMultiChannelAlarmConfig *c, const FbMultiChannelAlarmInput *in,
    FbMultiChannelAlarmOutput *o)
{
    bool edge, healthy=true;
    size_t i, selected=0;
    int32_t minimum=0, maximum=0;
    if (o==NULL) return FB_MULTI_ALARM_INVALID_ARGUMENT;
    *o=(FbMultiChannelAlarmOutput){0};
    o->status=FB_MULTI_ALARM_INVALID_ARGUMENT;
    if (s==NULL || c==NULL || in==NULL) return o->status;
    if (!in->enable) {
        FbMultiChannelAlarm_Init(s);
        o->status=FB_MULTI_ALARM_DISABLED;
        return o->status;
    }
    edge=in->reset && !s->reset_previous;
    s->reset_previous=in->reset;
    if (c->selected==NULL || c->count<2 || c->delay_ms>10000)
        return Invalid(s,o,FB_MULTI_ALARM_INVALID_CONFIG,edge);
    for (i=0;i<c->count;++i) if (c->selected[i]) ++selected;
    o->selected_count=selected;
    if (selected<2) return Invalid(s,o,FB_MULTI_ALARM_INVALID_CONFIG,edge);
    if (!s->configured || !Same(&s->config,c)) {
        FbMultiChannelAlarm_Init(s);
        s->config=*c;
        s->configured=true;
        s->reset_previous=in->reset;
    }
    if (in->samples==NULL || in->count!=c->count)
        return Invalid(s,o,FB_MULTI_ALARM_INVALID_INPUT,edge);
    selected=0;
    for (i=0;i<c->count;++i) {
        if (!c->selected[i]) continue;
        if (!in->samples[i].valid) healthy=false;
        if (selected==0 || in->samples[i].value<minimum) minimum=in->samples[i].value;
        if (selected==0 || in->samples[i].value>maximum) maximum=in->samples[i].value;
        ++selected;
    }
    if (!healthy) return Invalid(s,o,FB_MULTI_ALARM_INVALID_INPUT,edge);
    o->minimum=minimum; o->maximum=maximum;
    o->spread=(uint64_t)((int64_t)maximum-(int64_t)minimum);
    o->condition_active=o->spread>c->high_limit;
    o->valid=true;
    if (edge) {
        if (o->condition_active) o->reset_rejected=true;
        else {
            s->latched=false; s->fired=false;
            s->pending=false; s->elapsed_ms=0;
            o->reset_accepted=true;
        }
    }
    if (!o->condition_active) {
        s->pending=false; s->elapsed_ms=0;
        if (!c->hold) s->fired=false;
    } else {
        if (!s->pending) { s->pending=true; s->elapsed_ms=0; }
        else {
            uint32_t remaining=c->delay_ms-s->elapsed_ms;
            s->elapsed_ms += in->elapsed_ms>=remaining ? remaining : in->elapsed_ms;
        }
        o->qualified_active=s->elapsed_ms>=c->delay_ms;
        if (o->qualified_active && !s->fired) {
            s->fired=true;
            o->action_request=true;
            if (c->hold) s->latched=true;
        }
    }
    o->elapsed_ms=s->elapsed_ms;
    o->latched=s->latched;
    o->active=o->qualified_active || o->latched;
    o->status=FB_MULTI_ALARM_OK;
    return o->status;
}
