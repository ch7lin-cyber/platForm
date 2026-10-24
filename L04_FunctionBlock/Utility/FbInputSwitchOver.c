#include "FbInputSwitchOver.h"
#include <stddef.h>

void FbInputSwitchOver_Init(FbInputSwitchOver *instance)
{
    if (instance != NULL) {
        *instance = (FbInputSwitchOver){0};
        instance->selected = FB_SWITCH_NONE;
    }
}
static bool ConfigValid(const FbInputSwitchOverConfig *c)
{
    return c->sensor_low <= c->switch_low && c->switch_low <= c->switch_high &&
           c->switch_high <= c->sensor_high &&
           (c->fail_action == FB_SWITCH_USE_OTHER || c->fail_action == FB_SWITCH_KEEP_SOURCE);
}
static bool Same(const FbInputSwitchOverConfig *a, const FbInputSwitchOverConfig *b)
{
    return a->sensor_low==b->sensor_low && a->sensor_high==b->sensor_high &&
           a->switch_low==b->switch_low && a->switch_high==b->switch_high &&
           a->fail_action==b->fail_action;
}
FbSwitchStatus FbInputSwitchOver_Execute(FbInputSwitchOver *instance,
    const FbInputSwitchOverConfig *config, const FbInputSwitchOverInput *input,
    FbInputSwitchOverOutput *output)
{
    unsigned i;
    if (output == NULL) return FB_SWITCH_INVALID_ARGUMENT;
    *output = (FbInputSwitchOverOutput){0};
    output->selected = FB_SWITCH_NONE;
    output->status = FB_SWITCH_INVALID_ARGUMENT;
    if (instance==NULL || config==NULL || input==NULL) return output->status;
    if (!input->enable || input->reset) {
        FbInputSwitchOver_Init(instance);
        output->status = !input->enable ? FB_SWITCH_DISABLED : FB_SWITCH_RESET;
        return output->status;
    }
    if (!ConfigValid(config)) {
        FbInputSwitchOver_Init(instance);
        output->status = FB_SWITCH_INVALID_CONFIG;
        return output->status;
    }
    if (!instance->configured || !Same(&instance->config,config)) {
        instance->config = *config;
        instance->configured = true;
        instance->selected = FB_SWITCH_SOURCE1;
    }
    for (i=0;i<2;++i)
        if (!input->valid[i] || input->value[i]<config->sensor_low ||
            input->value[i]>config->sensor_high)
            output->error_mask |= (uint8_t)(1u<<i);
    if (output->error_mask==0) {
        instance->selected = input->value[0]<=config->switch_low ? FB_SWITCH_SOURCE1 :
            (input->value[1]>config->switch_high ? FB_SWITCH_SOURCE2 : FB_SWITCH_BOTH);
    } else if (config->fail_action==FB_SWITCH_USE_OTHER) {
        instance->selected = output->error_mask==1 ? FB_SWITCH_SOURCE2 :
            (output->error_mask==2 ? FB_SWITCH_SOURCE1 : FB_SWITCH_NONE);
    }
    output->selected = instance->selected;
    output->status = FB_SWITCH_INVALID_INPUT;
    if (instance->selected==FB_SWITCH_BOTH && output->error_mask==0) {
        output->value = (int32_t)(((int64_t)input->value[0]+input->value[1])/2);
        output->valid = true;
    } else if ((instance->selected==FB_SWITCH_SOURCE1 && !(output->error_mask&1)) ||
               (instance->selected==FB_SWITCH_SOURCE2 && !(output->error_mask&2))) {
        output->value = input->value[instance->selected];
        output->valid = true;
    }
    output->degraded = output->valid && output->error_mask!=0;
    if (output->valid) output->status = FB_SWITCH_OK;
    return output->status;
}
