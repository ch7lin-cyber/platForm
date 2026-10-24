#include "FbCascadeCoordinator.h"
#include <stddef.h>
#include <limits.h>

void FbCascadeCoordinator_Init(FbCascadeCoordinator *s)
{
    if (s!=NULL) *s=(FbCascadeCoordinator){0};
}
void FbCascadeCoordinator_DefaultConfig(FbCascadeConfig *c)
{
    if (c!=NULL) *c=(FbCascadeConfig){FB_CASCADE_DEVIATION,0,1000,-10000,60000,0,0,false};
}
bool FbCascadeCoordinator_ValidateConfig(const FbCascadeConfig *c)
{
    return c!=NULL && (c->mode==FB_CASCADE_ABSOLUTE || c->mode==FB_CASCADE_DEVIATION) &&
        c->mv_low<c->mv_high && c->sv_low<=c->sv_high;
}
static bool Same(const FbCascadeConfig *a, const FbCascadeConfig *b)
{
    return a->mode==b->mode && a->mv_low==b->mv_low && a->mv_high==b->mv_high &&
        a->sv_low==b->sv_low && a->sv_high==b->sv_high && a->offset==b->offset &&
        a->filter_tau_ms==b->filter_tau_ms && a->reverse_mapping==b->reverse_mapping;
}
FbCascadeStatus FbCascadeCoordinator_Execute(FbCascadeCoordinator *s,
    const FbCascadeConfig *c, const FbCascadeInput *in, FbCascadeOutput *o)
{
    int64_t low, high, mv, mapped, target;
    uint64_t position, span, width;
    if (o==NULL) return FB_CASCADE_INVALID_ARGUMENT;
    *o=(FbCascadeOutput){0}; o->status=FB_CASCADE_INVALID_ARGUMENT;
    if (s==NULL || c==NULL || in==NULL) return o->status;
    if (!in->enable || in->reset) {
        FbCascadeCoordinator_Init(s);
        o->status=!in->enable ? FB_CASCADE_DISABLED : FB_CASCADE_RESET;
        return o->status;
    }
    if (!FbCascadeCoordinator_ValidateConfig(c)) {
        FbCascadeCoordinator_Init(s); o->status=FB_CASCADE_INVALID_CONFIG; return o->status;
    }
    if (!in->mv_valid || (c->mode==FB_CASCADE_DEVIATION && !in->master_sv_valid)) {
        FbCascadeCoordinator_Init(s); o->status=FB_CASCADE_INVALID_INPUT; return o->status;
    }
    low=c->sv_low; high=c->sv_high;
    if (c->mode==FB_CASCADE_DEVIATION) { low+=in->master_sv; high+=in->master_sv; }
    if (low<INT32_MIN || high>INT32_MAX) {
        FbCascadeCoordinator_Init(s); o->status=FB_CASCADE_INVALID_INPUT; return o->status;
    }
    if (!s->configured || !Same(&s->config,c)) {
        FbCascadeCoordinator_Init(s); s->config=*c; s->configured=true;
    }
    mv=in->master_mv;
    if (mv<c->mv_low) { mv=c->mv_low; o->mv_clamped=true; }
    if (mv>c->mv_high) { mv=c->mv_high; o->mv_clamped=true; }
    position=(uint64_t)(mv-c->mv_low);
    span=(uint64_t)((int64_t)c->mv_high-c->mv_low);
    width=(uint64_t)(high-low);
    if (c->reverse_mapping) position=span-position;
    /* Each factor <= UINT32_MAX; product fits uint64 even for full ranges. */
    mapped=low+(int64_t)(position*width/span);
    target=mapped+c->offset;
    if (target<low) { target=low; o->target_clamped=true; }
    if (target>high) { target=high; o->target_clamped=true; }
    if (!s->seeded || c->filter_tau_ms==0) {
        s->filtered=(double)target; s->seeded=true;
    } else {
        double alpha=(double)in->elapsed_ms/((double)c->filter_tau_ms+in->elapsed_ms);
        s->filtered+=alpha*((double)target-s->filtered);
    }
    /* Moving deviation bounds also constrain retained filter history. */
    if (s->filtered<(double)low) { s->filtered=(double)low; o->filter_clamped=true; }
    if (s->filtered>(double)high) { s->filtered=(double)high; o->filter_clamped=true; }
    o->slave_sv=(int32_t)s->filtered;
    o->target_sv=(int32_t)target; o->limit_low=(int32_t)low; o->limit_high=(int32_t)high;
    o->at_low=o->slave_sv==o->limit_low; o->at_high=o->slave_sv==o->limit_high;
    o->valid=true; o->status=FB_CASCADE_OK;
    return o->status;
}
