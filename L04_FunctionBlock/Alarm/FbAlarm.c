#include "FbAlarm.h"

#include <stddef.h>

static bool IsRelativeMode(FbAlarmMode_t mode)
{
    return (mode == FB_ALARM_MODE_RELATIVE_HIGH_LOW) ||
           (mode == FB_ALARM_MODE_RELATIVE_HIGH) ||
           (mode == FB_ALARM_MODE_RELATIVE_LOW) ||
           (mode == FB_ALARM_MODE_HYSTERESIS_HIGH) ||
           (mode == FB_ALARM_MODE_HYSTERESIS_LOW);
}

static bool IsConfigurationValid(FbAlarmMode_t mode,
                                 int16_t alarm_high,
                                 int16_t alarm_low)
{
    if ((mode < FB_ALARM_MODE_DISABLED) || (mode >= FB_ALARM_MODE_COUNT))
    {
        return false;
    }
    if (IsRelativeMode(mode) &&
        ((alarm_high < 0) || (alarm_low < 0)))
    {
        return false;
    }
    if ((mode == FB_ALARM_MODE_ABSOLUTE_HIGH_LOW) &&
        (alarm_low > alarm_high))
    {
        return false;
    }
    return true;
}

static bool EvaluateAlarm(FbAlarm_t *instance,
                          int32_t process_value,
                          int32_t setpoint,
                          FbAlarmMode_t mode,
                          int32_t alarm_high,
                          int32_t alarm_low)
{
    switch (mode)
    {
        case FB_ALARM_MODE_RELATIVE_HIGH_LOW:
            return (process_value > (setpoint + alarm_high)) ||
                   (process_value < (setpoint - alarm_low));

        case FB_ALARM_MODE_RELATIVE_HIGH:
            return process_value > (setpoint + alarm_high);

        case FB_ALARM_MODE_RELATIVE_LOW:
            return process_value < (setpoint - alarm_low);

        case FB_ALARM_MODE_ABSOLUTE_HIGH_LOW:
            return (process_value > alarm_high) ||
                   (process_value < alarm_low);

        case FB_ALARM_MODE_ABSOLUTE_HIGH:
            return process_value > alarm_high;

        case FB_ALARM_MODE_ABSOLUTE_LOW:
            return process_value < alarm_low;

        case FB_ALARM_MODE_HYSTERESIS_HIGH:
            if (instance->hysteresis_active)
            {
                if (process_value < (setpoint + alarm_low))
                {
                    instance->hysteresis_active = false;
                }
            }
            else if (process_value > (setpoint + alarm_high))
            {
                instance->hysteresis_active = true;
            }
            return instance->hysteresis_active;

        case FB_ALARM_MODE_HYSTERESIS_LOW:
            if (instance->hysteresis_active)
            {
                if (process_value > (setpoint - alarm_low))
                {
                    instance->hysteresis_active = false;
                }
            }
            else if (process_value < (setpoint - alarm_high))
            {
                instance->hysteresis_active = true;
            }
            return instance->hysteresis_active;

        case FB_ALARM_MODE_DISABLED:
        default:
            instance->hysteresis_active = false;
            return false;
    }
}

void FbAlarm_Initialize(FbAlarm_t *instance,
                        int16_t initial_process_value)
{
    if (instance == NULL)
    {
        return;
    }

    instance->hold_latched = false;
    instance->standby_ready = false;
    instance->hysteresis_active = false;
    instance->peak_initialized = true;
    instance->peak_high = initial_process_value;
    instance->peak_low = initial_process_value;
}

bool FbAlarm_Execute(FbAlarm_t *instance,
                     int16_t process_value,
                     int16_t setpoint,
                     FbAlarmMode_t mode,
                     int16_t alarm_high,
                     int16_t alarm_low,
                     uint16_t clear_request,
                     uint16_t options,
                     FbAlarmOutput_t *output)
{
    bool alarm_active;
    bool standby_enabled;
    bool hold_enabled;
    int32_t process_value_32 = process_value;
    int32_t setpoint_32 = setpoint;

    if ((instance == NULL) || (output == NULL) ||
        !IsConfigurationValid(mode, alarm_high, alarm_low))
    {
        return false;
    }

    if ((clear_request & FB_ALARM_CLEAR_LATCH) != 0U)
    {
        instance->hold_latched = false;
        instance->hysteresis_active = false;
    }
    if (((clear_request & FB_ALARM_CLEAR_PEAK) != 0U) ||
        !instance->peak_initialized)
    {
        instance->peak_high = process_value;
        instance->peak_low = process_value;
        instance->peak_initialized = true;
    }

    if ((options & FB_ALARM_OPTION_PEAK) != 0U)
    {
        if (process_value > instance->peak_high)
        {
            instance->peak_high = process_value;
        }
        if (process_value < instance->peak_low)
        {
            instance->peak_low = process_value;
        }
    }

    standby_enabled = (options & FB_ALARM_OPTION_STANDBY) != 0U;
    if (!standby_enabled)
    {
        instance->standby_ready = true;
    }
    else if (!instance->standby_ready &&
             (process_value_32 >=
              (setpoint_32 - FB_ALARM_STANDBY_BAND)) &&
             (process_value_32 <=
              (setpoint_32 + FB_ALARM_STANDBY_BAND)))
    {
        instance->standby_ready = true;
    }

    if (instance->standby_ready)
    {
        alarm_active = EvaluateAlarm(instance, process_value_32, setpoint_32,
                                     mode, alarm_high, alarm_low);
    }
    else
    {
        alarm_active = false;
        instance->hysteresis_active = false;
    }

    hold_enabled = (options & FB_ALARM_OPTION_HOLD) != 0U;
    if (hold_enabled)
    {
        if (alarm_active)
        {
            instance->hold_latched = true;
        }
        alarm_active = instance->hold_latched;
    }
    else
    {
        instance->hold_latched = false;
    }

    if ((options & FB_ALARM_OPTION_INVERT) != 0U)
    {
        alarm_active = !alarm_active;
    }

    output->alarm_active = alarm_active;
    output->peak_high = instance->peak_high;
    output->peak_low = instance->peak_low;
    return true;
}
