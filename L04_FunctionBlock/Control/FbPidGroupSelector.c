#include "FbPidGroupSelector.h"
#include <stddef.h>

void FbPidGroupSelector_DefaultConfig(FbPidGroupSelectorConfig *c)
{
    unsigned i;
    if (c==NULL) return;
    *c=(FbPidGroupSelectorConfig){0};
    for (i=0;i<FB_PID_GROUP_COUNT;++i) {
        c->groups[i]=(FbPidGroupParameters){30000,30000,120000,30000,
                                           120000,30000,0,45,100,30};
    }
    for (i=0;i<FB_PID_GROUP_COUNT-1;++i) c->switch_sv[i]=1999900;
}
bool FbPidGroupSelector_ValidateConfig(const FbPidGroupSelectorConfig *c)
{
    unsigned i;
    if (c==NULL || c->selection>FB_PID_GROUP_AUTO) return false;
    for (i=1;i<FB_PID_GROUP_COUNT-1;++i)
        if (c->switch_sv[i]<c->switch_sv[i-1]) return false;
    for (i=0;i<FB_PID_GROUP_COUNT;++i) {
        const FbPidGroupParameters *p=&c->groups[i];
        if (p->pb==0 || p->pb>3000000 || p->cool_pb==0 || p->cool_pb>3000000 ||
            p->ti_ms>30000000 || p->td_ms>30000000 ||
            p->cool_ti_ms>30000000 || p->cool_td_ms>30000000 ||
            p->integral_preload>1000 || p->alpha>200 || p->beta>200 || p->gamma>200)
            return false;
    }
    return true;
}
FbPidGroupStatus FbPidGroupSelector_Execute(const FbPidGroupSelectorConfig *c,
    const FbPidGroupSelectorInput *in, FbPidGroupSelectorOutput *out)
{
    uint8_t group;
    if (out==NULL) return FB_PID_GROUP_INVALID_ARGUMENT;
    *out=(FbPidGroupSelectorOutput){0};
    out->status=FB_PID_GROUP_INVALID_ARGUMENT;
    if (c==NULL || in==NULL) return out->status;
    if (!in->enable) { out->status=FB_PID_GROUP_DISABLED; return out->status; }
    if (!FbPidGroupSelector_ValidateConfig(c)) {
        out->status=FB_PID_GROUP_INVALID_CONFIG; return out->status;
    }
    if (in->lock_group) {
        if (in->locked_group>=FB_PID_GROUP_COUNT) {
            out->status=FB_PID_GROUP_INVALID_INPUT; return out->status;
        }
        group=in->locked_group;
    } else if (c->selection==FB_PID_GROUP_AUTO) {
        if (!in->sv_valid) {
            out->status=FB_PID_GROUP_INVALID_INPUT; return out->status;
        }
        group=0;
        while (group<FB_PID_GROUP_COUNT-1 && in->sv>=c->switch_sv[group]) ++group;
    } else group=c->selection;
    out->parameters=c->groups[group];
    out->group=group;
    out->locked=in->lock_group;
    out->valid=true;
    out->status=FB_PID_GROUP_OK;
    return out->status;
}
