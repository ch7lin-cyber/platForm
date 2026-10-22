#include "FbLogic.h"

#include <stddef.h>
#include <string.h>

static void ClearRuntime(FbLogic_t *instance)
{
    instance->value = 0U;
    instance->delay_elapsed_ms = 0U;
    instance->delay_active = false;
}

void FbLogic_Initialize(FbLogic_t *instance)
{
    if (instance != NULL)
        (void)memset(instance, 0, sizeof(*instance));
}

void FbLogic_DefaultConfig(FbLogicConfig_t *config)
{
    if (config != NULL) (void)memset(config, 0, sizeof(*config));
}

static bool ConfigChanged(const FbLogicConfig_t *a, const FbLogicConfig_t *b)
{
    uint8_t i;
    if (a->operator_type != b->operator_type ||
        a->output_inverted != b->output_inverted ||
        a->on_delay_ms != b->on_delay_ms || a->off_delay_ms != b->off_delay_ms)
        return true;
    for (i = 0U; i < FB_LOGIC_INPUT_COUNT; ++i)
        if (((a->input_inversions[i] ^ b->input_inversions[i]) & 1U) != 0U)
            return true;
    return false;
}

static uint16_t Evaluate(const FbLogicConfig_t *config, const FbLogicInput_t *input)
{
    uint16_t v[FB_LOGIC_INPUT_COUNT];
    uint16_t result = 0U;
    uint8_t i;
    for (i = 0U; i < FB_LOGIC_INPUT_COUNT; ++i)
        v[i] = (input->inputs[i] ^ config->input_inversions[i]) & 1U;
    switch (config->operator_type)
    {
        case FB_LOGIC_OPERATOR_AND: result = v[0] & v[1] & v[2] & v[3]; break;
        case FB_LOGIC_OPERATOR_OR: result = v[0] | v[1] | v[2] | v[3]; break;
        case FB_LOGIC_OPERATOR_XOR: result = v[0] ^ v[1] ^ v[2] ^ v[3]; break;
        case FB_LOGIC_OPERATOR_OFF:
        default: break;
    }
    return config->output_inverted ? (result ^ 1U) : result;
}

static void ApplyDelay(FbLogic_t *instance, uint16_t target, uint32_t elapsed_ms)
{
    uint32_t limit = target ? instance->config.on_delay_ms :
                              instance->config.off_delay_ms;
    if (target == instance->value)
    {
        instance->delay_active = false;
        instance->delay_elapsed_ms = 0U;
    }
    else if (limit == 0U)
    {
        instance->value = target;
        instance->delay_active = false;
        instance->delay_elapsed_ms = 0U;
    }
    else if (!instance->delay_active)
    {
        instance->delay_active = true;
        instance->delay_elapsed_ms = 0U;
    }
    else if (elapsed_ms >= limit - instance->delay_elapsed_ms)
    {
        /* Compare remaining time before addition, avoiding uint32 overflow. */
        instance->value = target;
        instance->delay_active = false;
        instance->delay_elapsed_ms = 0U;
    }
    else instance->delay_elapsed_ms += elapsed_ms;
}

FbLogicStatus_t FbLogic_Execute(FbLogic_t *instance,
                                const FbLogicConfig_t *config,
                                const FbLogicInput_t *input,
                                FbLogicOutput_t *output)
{
    bool reset_edge;
    if (output != NULL)
    {
        (void)memset(output, 0, sizeof(*output));
        output->status = FB_LOGIC_STATUS_INVALID_ARGUMENT;
    }
    if (instance == NULL || input == NULL || output == NULL)
    {
        if (instance != NULL) FbLogic_Initialize(instance);
        return FB_LOGIC_STATUS_INVALID_ARGUMENT;
    }
    reset_edge = input->reset && !instance->reset_previous;
    instance->reset_previous = input->reset;
    if (!input->enable)
    {
        ClearRuntime(instance);
        instance->configured = false;
        output->status = FB_LOGIC_STATUS_DISABLED;
        return output->status;
    }
    if (config == NULL)
    {
        ClearRuntime(instance);
        instance->configured = false;
        return output->status;
    }
    if (config->operator_type < FB_LOGIC_OPERATOR_OFF ||
        config->operator_type > FB_LOGIC_OPERATOR_XOR)
    {
        ClearRuntime(instance);
        instance->configured = false;
        output->status = FB_LOGIC_STATUS_INVALID_CONFIGURATION;
        return output->status;
    }
    if (config->operator_type == FB_LOGIC_OPERATOR_OFF)
    {
        ClearRuntime(instance);
        instance->configured = false;
        output->status = FB_LOGIC_STATUS_DISABLED;
        return output->status;
    }
    if (!instance->configured || ConfigChanged(&instance->config, config))
    {
        ClearRuntime(instance);
        instance->config = *config;
        instance->configured = true;
    }
    if (reset_edge)
    {
        ClearRuntime(instance);
        output->status = FB_LOGIC_STATUS_RESET;
        return output->status;
    }
    output->raw_value = Evaluate(config, input);
    ApplyDelay(instance, output->raw_value, input->elapsed_ms);
    output->value = instance->value;
    output->delay_active = instance->delay_active;
    output->on_delay_active = instance->delay_active && output->raw_value != 0U;
    output->off_delay_active = instance->delay_active && output->raw_value == 0U;
    output->delay_elapsed_ms = instance->delay_elapsed_ms;
    output->status = FB_LOGIC_STATUS_OK;
    return output->status;
}
