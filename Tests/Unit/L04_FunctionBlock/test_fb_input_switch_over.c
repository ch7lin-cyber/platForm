#include "FbInputSwitchOver.h"
#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>

int main(void)
{
    FbInputSwitchOver a={0}, b={0};
    FbInputSwitchOverConfig c={-100000,1000000,200000,400000,FB_SWITCH_USE_OTHER};
    FbInputSwitchOverInput in={{200000,900000},{true,true},true,false};
    FbInputSwitchOverOutput out;
    assert(FbInputSwitchOver_Execute(&a,&c,&in,&out)==FB_SWITCH_OK);
    assert(out.selected==FB_SWITCH_SOURCE1 && out.value==200000);
    in.value[0]=200001; in.value[1]=400000;
    assert(FbInputSwitchOver_Execute(&a,&c,&in,&out)==FB_SWITCH_OK);
    assert(out.selected==FB_SWITCH_BOTH && out.value==300000);
    in.value[1]=400001;
    assert(FbInputSwitchOver_Execute(&a,&c,&in,&out)==FB_SWITCH_OK);
    assert(out.selected==FB_SWITCH_SOURCE2 && out.value==400001);
    /* All fault masks, including recovery, for both policies. */
    for (int policy=0;policy<2;++policy) {
        c.fail_action=(FbSwitchFailAction)policy;
        for (unsigned mask=0;mask<4;++mask) {
            FbInputSwitchOver_Init(&a);
            in.value[0]=300000; in.value[1]=350000;
            in.valid[0]=in.valid[1]=true;
            assert(FbInputSwitchOver_Execute(&a,&c,&in,&out)==FB_SWITCH_OK);
            assert(out.selected==FB_SWITCH_BOTH);
            in.valid[0]=!(mask&1); in.valid[1]=!(mask&2);
            FbInputSwitchOver_Execute(&a,&c,&in,&out);
            assert(out.error_mask==mask);
            assert(out.valid==(mask==0 || (policy==0 && mask!=3)));
            if (out.valid && mask) assert(out.degraded);
            if (!out.valid) assert(out.value==0 && !out.degraded);
            in.valid[0]=in.valid[1]=true;
            assert(FbInputSwitchOver_Execute(&a,&c,&in,&out)==FB_SWITCH_OK);
            assert(out.selected==FB_SWITCH_BOTH && !out.degraded);
        }
    }
    /* KEEP never replaces an invalid selected source with its last PV. */
    in.value[0]=100000;
    FbInputSwitchOver_Execute(&a,&c,&in,&out);
    in.valid[0]=false;
    assert(FbInputSwitchOver_Execute(&a,&c,&in,&out)==FB_SWITCH_INVALID_INPUT);
    assert(out.selected==FB_SWITCH_SOURCE1 && out.value==0);
    /* Independent instance, initial KEEP_SOURCE chooses SOURCE1. */
    assert(FbInputSwitchOver_Execute(&b,&c,&in,&out)==FB_SWITCH_INVALID_INPUT);
    assert(out.selected==FB_SWITCH_SOURCE1);
    in.valid[0]=true; in.valid[1]=false;
    assert(FbInputSwitchOver_Execute(&b,&c,&in,&out)==FB_SWITCH_OK);
    assert(out.degraded && out.value==100000);
    /* Config change resets a prior SOURCE2 selection before failure. */
    in.valid[1]=true; in.value[0]=300000; in.value[1]=500000;
    FbInputSwitchOver_Execute(&b,&c,&in,&out);
    assert(out.selected==FB_SWITCH_SOURCE2);
    c.switch_low=200001; in.valid[0]=false;
    assert(FbInputSwitchOver_Execute(&b,&c,&in,&out)==FB_SWITCH_INVALID_INPUT);
    assert(out.selected==FB_SWITCH_SOURCE1);
    /* Sensor limits inclusive; outside range is invalid, never clamped. */
    c.fail_action=FB_SWITCH_USE_OTHER;
    in.valid[0]=in.valid[1]=true; in.value[0]=c.sensor_low; in.value[1]=c.sensor_high;
    FbInputSwitchOver_Execute(&a,&c,&in,&out); assert(out.error_mask==0);
    in.value[0]=c.sensor_low-1;
    FbInputSwitchOver_Execute(&a,&c,&in,&out);
    assert(out.selected==FB_SWITCH_SOURCE2 && out.error_mask==1 && out.degraded);
    /* Full int32 average and negative odd sum round toward zero. */
    c=(FbInputSwitchOverConfig){INT_MIN,INT_MAX,INT_MIN,INT_MAX,FB_SWITCH_USE_OTHER};
    in.value[0]=INT_MAX; in.value[1]=INT_MAX;
    FbInputSwitchOver_Execute(&a,&c,&in,&out); assert(out.value==INT_MAX);
    in.value[0]=-3; in.value[1]=-2;
    FbInputSwitchOver_Execute(&a,&c,&in,&out); assert(out.value==-2);
    /* Equal thresholds valid and deterministic. */
    c.switch_low=c.switch_high=0; in.value[0]=1; in.value[1]=0;
    FbInputSwitchOver_Execute(&a,&c,&in,&out); assert(out.selected==FB_SWITCH_BOTH);
    c.switch_low=1; c.switch_high=0;
    assert(FbInputSwitchOver_Execute(&a,&c,&in,&out)==FB_SWITCH_INVALID_CONFIG);
    assert(!a.configured && !out.valid);
    c.switch_low=0; c.fail_action=(FbSwitchFailAction)99;
    assert(FbInputSwitchOver_Execute(&a,&c,&in,&out)==FB_SWITCH_INVALID_CONFIG);
    in.enable=false;
    assert(FbInputSwitchOver_Execute(&a,&c,&in,&out)==FB_SWITCH_DISABLED);
    assert(out.value==0 && out.selected==FB_SWITCH_NONE && !a.configured);
    in.enable=true; in.reset=true;
    assert(FbInputSwitchOver_Execute(&a,&c,&in,&out)==FB_SWITCH_RESET);
    assert(FbInputSwitchOver_Execute(NULL,&c,&in,&out)==FB_SWITCH_INVALID_ARGUMENT);
    assert(FbInputSwitchOver_Execute(&a,NULL,&in,&out)==FB_SWITCH_INVALID_ARGUMENT);
    assert(FbInputSwitchOver_Execute(&a,&c,NULL,&out)==FB_SWITCH_INVALID_ARGUMENT);
    assert(FbInputSwitchOver_Execute(&a,&c,&in,NULL)==FB_SWITCH_INVALID_ARGUMENT);
    puts("FbInputSwitchOver: selection, fault policies, recovery, range, lifecycle and arithmetic PASS");
    return 0;
}
