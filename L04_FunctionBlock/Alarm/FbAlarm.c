#include "FbAlarm.h"

#include <stddef.h>
#include <string.h>

static void ClearRuntime(FbAlarm_t *instance)
{
    instance->alarm_latched = false;
    instance->standby_ready = false;
    instance->hysteresis_active = false;
    instance->delay_active = false;
    instance->delay_elapsed_ms = 0U;
}

static void ClearPeak(FbAlarm_t *instance)
{
    instance->peak_valid = false;
    instance->peak_high = 0;
    instance->peak_low = 0;
}

void FbAlarm_Initialize(FbAlarm_t *instance)
{
    if (instance != NULL) (void)memset(instance, 0, sizeof(*instance));
}

void FbAlarm_DefaultConfig(FbAlarmConfig_t *config)
{
    if (config != NULL)
    {
        (void)memset(config, 0, sizeof(*config));
        config->standby_band = FB_ALARM_STANDBY_BAND_DEFAULT;
    }
}

static bool ValidConfig(const FbAlarmConfig_t *config)
{
    if (config == NULL || config->mode < FB_ALARM_MODE_DISABLED ||
        config->mode >= FB_ALARM_MODE_COUNT ||
        (config->options & (uint16_t)~FB_ALARM_OPTION_MASK) != 0U ||
        config->delay_ms > FB_ALARM_DELAY_MAX_MS ||
        config->reset_mode < FB_ALARM_RESET_AUTO ||
        config->reset_mode >= FB_ALARM_RESET_MODE_COUNT) return false;
    if ((config->options & FB_ALARM_OPTION_STANDBY) != 0U &&
        (config->standby_band < 0 ||
         config->standby_band > FB_ALARM_STANDBY_BAND_MAX)) return false;
    if (config->mode == FB_ALARM_MODE_DISABLED) return true;
    if (config->alarm_high < FB_ALARM_LIMIT_MIN || config->alarm_high > FB_ALARM_LIMIT_MAX ||
        config->alarm_low < FB_ALARM_LIMIT_MIN || config->alarm_low > FB_ALARM_LIMIT_MAX)
        return false;
    if ((config->mode == FB_ALARM_MODE_ABSOLUTE_HIGH_LOW ||
         config->mode == FB_ALARM_MODE_HYSTERESIS_HIGH ||
         config->mode == FB_ALARM_MODE_HYSTERESIS_LOW) &&
        config->alarm_low > config->alarm_high) return false;
    if (config->mode == FB_ALARM_MODE_RELATIVE_HIGH_LOW &&
        (int64_t)config->alarm_high + config->alarm_low < 0) return false;
    return true;
}

static bool ConfigChanged(const FbAlarmConfig_t *a, const FbAlarmConfig_t *b)
{
    return a->mode != b->mode || a->reset_mode != b->reset_mode || a->alarm_high != b->alarm_high ||
           a->alarm_low != b->alarm_low || a->delay_ms != b->delay_ms ||
           a->options != b->options || a->standby_band != b->standby_band;
}

static bool Evaluate(FbAlarm_t *instance, const FbAlarmInput_t *input)
{
    int64_t pv = input->process_value;
    int64_t sv = input->setpoint;
    int64_t high = instance->config.alarm_high;
    int64_t low = instance->config.alarm_low;
    switch (instance->config.mode)
    {
        case FB_ALARM_MODE_RELATIVE_HIGH_LOW: return pv > sv + high || pv < sv - low;
        case FB_ALARM_MODE_RELATIVE_HIGH: return pv > sv + high;
        case FB_ALARM_MODE_RELATIVE_LOW: return pv < sv - low;
        case FB_ALARM_MODE_ABSOLUTE_HIGH_LOW: return pv > high || pv < low;
        case FB_ALARM_MODE_ABSOLUTE_HIGH: return pv > high;
        case FB_ALARM_MODE_ABSOLUTE_LOW: return pv < low;
        case FB_ALARM_MODE_HYSTERESIS_HIGH:
            if (instance->hysteresis_active)
            {
                if (pv < sv + low) instance->hysteresis_active = false;
            }
            else if (pv > sv + high) instance->hysteresis_active = true;
            return instance->hysteresis_active;
        case FB_ALARM_MODE_HYSTERESIS_LOW:
            if (instance->hysteresis_active)
            {
                if (pv > sv - low) instance->hysteresis_active = false;
            }
            else if (pv < sv - high) instance->hysteresis_active = true;
            return instance->hysteresis_active;
        default: return false;
    }
}

static bool Qualify(FbAlarm_t *instance, bool condition, uint32_t elapsed_ms)
{
    uint32_t limit = instance->config.delay_ms;
    if (!condition)
    {
        instance->delay_active = false;
        instance->delay_elapsed_ms = 0U;
        return false;
    }
    if (limit == 0U) return true;
    if (!instance->delay_active)
    {
        instance->delay_active = true;
        instance->delay_elapsed_ms = 0U;
    }
    else if (elapsed_ms >= limit - instance->delay_elapsed_ms)
        instance->delay_elapsed_ms = limit;
    else instance->delay_elapsed_ms += elapsed_ms;
    return instance->delay_elapsed_ms == limit;
}

static FbAlarmStatus_t Publish(const FbAlarm_t *instance,
                               FbAlarmOutput_t *output, FbAlarmStatus_t status)
{
    output->alarm_latched = instance->alarm_latched;
    output->standby_ready = instance->standby_ready;
    output->delay_active = instance->delay_active &&
                           instance->delay_elapsed_ms < instance->config.delay_ms;
    output->delay_elapsed_ms = instance->delay_elapsed_ms;
    output->peak_valid = instance->peak_valid;
    output->peak_high = instance->peak_high;
    output->peak_low = instance->peak_low;
    output->status = status;
    return status;
}

FbAlarmStatus_t FbAlarm_Execute(FbAlarm_t *instance,
                                const FbAlarmConfig_t *config,
                                const FbAlarmInput_t *input,
                                FbAlarmOutput_t *output)
{
    bool reset_edge;
    bool peak_edge;
    bool qualified;
    bool active;
    FbAlarmStatus_t status = FB_ALARM_STATUS_OK;
    if (output != NULL)
    {
        (void)memset(output, 0, sizeof(*output));
        output->status = FB_ALARM_STATUS_INVALID_ARGUMENT;
    }
    if (instance == NULL) return FB_ALARM_STATUS_INVALID_ARGUMENT;
    if (input == NULL || output == NULL)
    {
        ClearRuntime(instance);
        instance->reset_previous = false;
        instance->clear_peak_previous = false;
        if (output != NULL) (void)Publish(instance, output, FB_ALARM_STATUS_INVALID_ARGUMENT);
        return FB_ALARM_STATUS_INVALID_ARGUMENT;
    }
    reset_edge = input->reset && !instance->reset_previous;
    peak_edge = input->clear_peak && !instance->clear_peak_previous;
    instance->reset_previous = input->reset;
    instance->clear_peak_previous = input->clear_peak;
    if (peak_edge) ClearPeak(instance);
    if (!input->enable)
    {
        ClearRuntime(instance);
        return Publish(instance, output, FB_ALARM_STATUS_DISABLED);
    }
    if (!ValidConfig(config))
    {
        ClearRuntime(instance);
        return Publish(instance, output, FB_ALARM_STATUS_INVALID_CONFIGURATION);
    }
    if (!instance->configured || ConfigChanged(&instance->config, config))
    {
        ClearRuntime(instance);
        ClearPeak(instance);
        instance->config = *config;
        instance->configured = true;
    }
    if (!input->valid)
    {
        ClearRuntime(instance);
        return Publish(instance, output, FB_ALARM_STATUS_INVALID_INPUT);
    }
    if (config->mode == FB_ALARM_MODE_DISABLED)
    {
        ClearRuntime(instance);
        return Publish(instance, output, FB_ALARM_STATUS_DISABLED);
    }
    if ((config->options & FB_ALARM_OPTION_STANDBY) == 0U ||
        ((int64_t)input->process_value >= (int64_t)input->setpoint - config->standby_band &&
         (int64_t)input->process_value <= (int64_t)input->setpoint + config->standby_band))
        instance->standby_ready = true;
    output->condition_active = instance->standby_ready && Evaluate(instance, input);
    qualified = Qualify(instance, output->condition_active, input->elapsed_ms);
    output->qualified_active = qualified;
    if (config->reset_mode == FB_ALARM_RESET_MANUAL)
    {
        if (qualified) instance->alarm_latched = true;
    }
    else instance->alarm_latched = false;
    if (reset_edge)
    {
        if (output->condition_active)
            status = FB_ALARM_STATUS_RESET_REJECTED;
        else
        {
            instance->alarm_latched = false;
            status = FB_ALARM_STATUS_RESET;
        }
    }
    if ((config->options & FB_ALARM_OPTION_PEAK) != 0U && qualified)
    {
        if (!instance->peak_valid)
        {
            instance->peak_high = input->process_value;
            instance->peak_low = input->process_value;
            instance->peak_valid = true;
        }
        else
        {
            if (input->process_value > instance->peak_high) instance->peak_high = input->process_value;
            if (input->process_value < instance->peak_low) instance->peak_low = input->process_value;
        }
    }
    active = qualified || instance->alarm_latched;
    output->alarm_active = (config->options & FB_ALARM_OPTION_INVERT) != 0U ? !active : active;
    return Publish(instance, output, status);
}
