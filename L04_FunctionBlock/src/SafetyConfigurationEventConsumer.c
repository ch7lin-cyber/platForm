#include "SafetyConfigurationEventConsumer.h"

#include <stddef.h>
#include <string.h>

static SafetyConfigurationRange_t
    g_safety_ranges[EVENT_SERVICE_TEMPERATURE_INPUT_COUNT];
static SafetySensorRangeResolver_t g_range_resolver;
static void *g_range_resolver_context;

bool SafetyConfigurationEventConsumer_Initialize(
    SafetySensorRangeResolver_t range_resolver,
    void *range_resolver_context)
{
    uint8_t channel;

    if (range_resolver == NULL)
    {
        return false;
    }

    (void)memset(g_safety_ranges, 0, sizeof(g_safety_ranges));
    for (channel = 0U;
         channel < EVENT_SERVICE_TEMPERATURE_INPUT_COUNT;
         channel++)
    {
        /* No validated sensor configuration means outputs remain inhibited. */
        g_safety_ranges[channel].output_inhibit = true;
    }
    g_range_resolver = range_resolver;
    g_range_resolver_context = range_resolver_context;
    return true;
}

bool SafetyConfigurationEventConsumer_Process(uint8_t channel)
{
    TemperatureInputConfigurationChangedEvent_t event;
    SafetyConfigurationRange_t new_range;
    const uint32_t safety_relevant_changes =
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

    new_range = g_safety_ranges[channel];
    if ((event.changed_mask & safety_relevant_changes) != 0U)
    {
        /* Active configuration changed: fail safe until it is validated. */
        g_safety_ranges[channel].output_inhibit = true;
        if (!g_range_resolver(&event.new_configuration,
                              &new_range.measurement_minimum,
                              &new_range.measurement_maximum,
                              &new_range.input_enabled,
                              g_range_resolver_context) ||
            (new_range.input_enabled &&
             !(new_range.measurement_minimum <=
               new_range.measurement_maximum)))
        {
            return false;
        }

        new_range.valid = true;
        new_range.output_inhibit = !new_range.input_enabled;
    }

    new_range.configuration_revision = event.configuration_revision;
    g_safety_ranges[channel] = new_range;
    return EventService_Acknowledge(event.event_id, EVENT_ACK_SAFETY);
}

bool SafetyConfigurationEventConsumer_GetRange(
    uint8_t channel,
    SafetyConfigurationRange_t *range)
{
    if ((channel >= EVENT_SERVICE_TEMPERATURE_INPUT_COUNT) ||
        (range == NULL))
    {
        return false;
    }

    *range = g_safety_ranges[channel];
    return range->valid;
}

bool SafetyConfigurationEventConsumer_IsOutputInhibited(uint8_t channel)
{
    return (channel >= EVENT_SERVICE_TEMPERATURE_INPUT_COUNT) ||
           (g_range_resolver == NULL) ||
           g_safety_ranges[channel].output_inhibit;
}
