#include "AlarmConfigurationEventConsumer.h"

#include <stddef.h>
#include <string.h>

static AlarmConfigurationRange_t
    g_alarm_ranges[EVENT_SERVICE_TEMPERATURE_INPUT_COUNT];
static AlarmSensorRangeResolver_t g_range_resolver;
static void *g_range_resolver_context;

bool AlarmConfigurationEventConsumer_Initialize(
    AlarmSensorRangeResolver_t range_resolver,
    void *range_resolver_context)
{
    if (range_resolver == NULL)
    {
        return false;
    }

    (void)memset(g_alarm_ranges, 0, sizeof(g_alarm_ranges));
    g_range_resolver = range_resolver;
    g_range_resolver_context = range_resolver_context;
    return true;
}

bool AlarmConfigurationEventConsumer_Process(uint8_t channel)
{
    TemperatureInputConfigurationChangedEvent_t event;
    AlarmConfigurationRange_t new_range;
    const uint32_t alarm_relevant_changes =
        EVENT_TEMPERATURE_INPUT_CHANGE_SENSOR_TYPE |
        EVENT_TEMPERATURE_INPUT_CHANGE_TC_LINEARIZATION;

    if ((channel >= EVENT_SERVICE_TEMPERATURE_INPUT_COUNT) ||
        (g_range_resolver == NULL))
    {
        return false;
    }

    if (!EventService_GetTemperatureInputConfigurationChanged(
            channel, &event))
    {
        return true;
    }

    new_range = g_alarm_ranges[channel];
    if ((event.changed_mask & alarm_relevant_changes) != 0U)
    {
        if (!g_range_resolver(&event.new_configuration,
                              &new_range.setpoint_minimum,
                              &new_range.setpoint_maximum,
                              &new_range.input_enabled,
                              g_range_resolver_context) ||
            (new_range.input_enabled &&
             !(new_range.setpoint_minimum <= new_range.setpoint_maximum)))
        {
            /* Do not ACK: EventService retains the event for retry/fault logic. */
            return false;
        }
        new_range.valid = true;
    }

    new_range.configuration_revision = event.configuration_revision;
    g_alarm_ranges[channel] = new_range;
    return EventService_Acknowledge(event.event_id, EVENT_ACK_ALARM);
}

bool AlarmConfigurationEventConsumer_GetRange(
    uint8_t channel,
    AlarmConfigurationRange_t *range)
{
    if ((channel >= EVENT_SERVICE_TEMPERATURE_INPUT_COUNT) ||
        (range == NULL))
    {
        return false;
    }

    *range = g_alarm_ranges[channel];
    return range->valid;
}
