#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "AlarmConfigurationEventConsumer.h"

static bool g_resolver_succeeds;
static uint32_t g_resolver_call_count;

static bool ResolveRange(
    const EventTemperatureInputConfiguration_t *configuration,
    float *minimum,
    float *maximum,
    bool *input_enabled,
    void *context)
{
    (void)context;
    g_resolver_call_count++;
    if (!g_resolver_succeeds || (configuration->sensor_type != 95U))
    {
        return false;
    }

    *minimum = -270.0F;
    *maximum = 1372.0F;
    *input_enabled = true;
    return true;
}

static void TestAlarmRangeUpdateAndAcknowledge(void)
{
    EventTemperatureInputConfiguration_t old_configuration =
        {0.5F, 62U, 46U};
    EventTemperatureInputConfiguration_t new_configuration =
        {0.5F, 95U, 48U};
    AlarmConfigurationRange_t range;

    assert(EventService_Initialize(EVENT_ACK_SERIAL_REQUIRED_DEFAULT));
    assert(EventService_ConfigureTemperatureInputRequiredAckMask(
        EVENT_ACK_ALARM));
    g_resolver_succeeds = true;
    g_resolver_call_count = 0U;
    assert(AlarmConfigurationEventConsumer_Initialize(ResolveRange, NULL));
    assert(EventService_RaiseTemperatureInputConfigurationChanged(
        0U, 1U,
        EVENT_TEMPERATURE_INPUT_CHANGE_SENSOR_TYPE |
            EVENT_TEMPERATURE_INPUT_CHANGE_TC_LINEARIZATION,
        &old_configuration, &new_configuration, NULL));

    assert(AlarmConfigurationEventConsumer_Process(0U));
    assert(g_resolver_call_count == 1U);
    assert(!EventService_IsTemperatureInputConfigurationChangedPending(0U));
    assert(AlarmConfigurationEventConsumer_GetRange(0U, &range));
    assert(range.setpoint_minimum == -270.0F);
    assert(range.setpoint_maximum == 1372.0F);
    assert(range.input_enabled);
    assert(range.configuration_revision == 1U);
}

static void TestResolverFailureRetainsEvent(void)
{
    EventTemperatureInputConfiguration_t old_configuration =
        {0.5F, 95U, 48U};
    EventTemperatureInputConfiguration_t new_configuration =
        {0.5F, 95U, 46U};

    g_resolver_succeeds = false;
    assert(EventService_RaiseTemperatureInputConfigurationChanged(
        0U, 2U, EVENT_TEMPERATURE_INPUT_CHANGE_TC_LINEARIZATION,
        &old_configuration, &new_configuration, NULL));
    assert(!AlarmConfigurationEventConsumer_Process(0U));
    assert(EventService_IsTemperatureInputConfigurationChangedPending(0U));

    g_resolver_succeeds = true;
    assert(AlarmConfigurationEventConsumer_Process(0U));
    assert(!EventService_IsTemperatureInputConfigurationChangedPending(0U));
}

static void TestFilterOnlyChangeDoesNotResolveRangeAgain(void)
{
    EventTemperatureInputConfiguration_t old_configuration =
        {0.5F, 95U, 46U};
    EventTemperatureInputConfiguration_t new_configuration =
        {2.0F, 95U, 46U};
    AlarmConfigurationRange_t range;
    uint32_t calls_before = g_resolver_call_count;

    assert(EventService_RaiseTemperatureInputConfigurationChanged(
        0U, 3U, EVENT_TEMPERATURE_INPUT_CHANGE_FILTER_TIME_CONSTANT,
        &old_configuration, &new_configuration, NULL));
    assert(AlarmConfigurationEventConsumer_Process(0U));
    assert(g_resolver_call_count == calls_before);
    assert(AlarmConfigurationEventConsumer_GetRange(0U, &range));
    assert(range.configuration_revision == 3U);
}

int main(void)
{
    TestAlarmRangeUpdateAndAcknowledge();
    TestResolverFailureRetainsEvent();
    TestFilterOnlyChangeDoesNotResolveRangeAgain();
    return 0;
}
