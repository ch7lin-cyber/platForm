#include <assert.h>
#include <stdbool.h>

#include "SafetyConfigurationEventConsumer.h"

static bool g_resolver_succeeds;

static bool ResolveRange(
    const EventTemperatureInputConfiguration_t *configuration,
    float *minimum,
    float *maximum,
    bool *input_enabled,
    void *context)
{
    (void)context;
    if (!g_resolver_succeeds)
    {
        return false;
    }

    if (configuration->sensor_type == 62U)
    {
        *minimum = 0.0F;
        *maximum = 0.0F;
        *input_enabled = false;
        return true;
    }
    if (configuration->sensor_type == 95U)
    {
        *minimum = -270.0F;
        *maximum = 1372.0F;
        *input_enabled = true;
        return true;
    }
    return false;
}

static void TestEnabledSensorUpdatesSafetyRange(void)
{
    EventTemperatureInputConfiguration_t old_configuration =
        {0.5F, 62U, 46U};
    EventTemperatureInputConfiguration_t new_configuration =
        {0.5F, 95U, 48U};
    SafetyConfigurationRange_t range;

    assert(EventService_Initialize(EVENT_ACK_SERIAL_REQUIRED_DEFAULT));
    assert(EventService_ConfigureTemperatureInputRequiredAckMask(
        EVENT_ACK_SAFETY));
    g_resolver_succeeds = true;
    assert(SafetyConfigurationEventConsumer_Initialize(ResolveRange, NULL));
    assert(SafetyConfigurationEventConsumer_IsOutputInhibited(0U));
    assert(EventService_RaiseTemperatureInputConfigurationChanged(
        0U, 1U, EVENT_TEMPERATURE_INPUT_CHANGE_SENSOR_TYPE,
        &old_configuration, &new_configuration, NULL));
    assert(SafetyConfigurationEventConsumer_Process(0U));
    assert(!EventService_IsTemperatureInputConfigurationChangedPending(0U));
    assert(SafetyConfigurationEventConsumer_GetRange(0U, &range));
    assert(range.input_enabled);
    assert(!range.output_inhibit);
    assert(range.measurement_minimum == -270.0F);
    assert(range.measurement_maximum == 1372.0F);
    assert(range.configuration_revision == 1U);
}

static void TestSensorOffInhibitsOutput(void)
{
    EventTemperatureInputConfiguration_t old_configuration =
        {0.5F, 95U, 48U};
    EventTemperatureInputConfiguration_t new_configuration =
        {0.5F, 62U, 48U};
    SafetyConfigurationRange_t range;

    assert(EventService_RaiseTemperatureInputConfigurationChanged(
        0U, 2U, EVENT_TEMPERATURE_INPUT_CHANGE_SENSOR_TYPE,
        &old_configuration, &new_configuration, NULL));
    assert(SafetyConfigurationEventConsumer_Process(0U));
    assert(SafetyConfigurationEventConsumer_GetRange(0U, &range));
    assert(!range.input_enabled);
    assert(range.output_inhibit);
    assert(SafetyConfigurationEventConsumer_IsOutputInhibited(0U));
}

static void TestResolverFailureRetainsEventAndInhibitsOutput(void)
{
    EventTemperatureInputConfiguration_t old_configuration =
        {0.5F, 62U, 48U};
    EventTemperatureInputConfiguration_t new_configuration =
        {0.5F, 95U, 48U};

    g_resolver_succeeds = false;
    assert(EventService_RaiseTemperatureInputConfigurationChanged(
        0U, 3U, EVENT_TEMPERATURE_INPUT_CHANGE_SENSOR_TYPE,
        &old_configuration, &new_configuration, NULL));
    assert(!SafetyConfigurationEventConsumer_Process(0U));
    assert(EventService_IsTemperatureInputConfigurationChangedPending(0U));
    assert(SafetyConfigurationEventConsumer_IsOutputInhibited(0U));

    g_resolver_succeeds = true;
    assert(SafetyConfigurationEventConsumer_Process(0U));
    assert(!EventService_IsTemperatureInputConfigurationChangedPending(0U));
    assert(!SafetyConfigurationEventConsumer_IsOutputInhibited(0U));
}

int main(void)
{
    TestEnabledSensorUpdatesSafetyRange();
    TestSensorOffInhibitsOutput();
    TestResolverFailureRetainsEventAndInhibitsOutput();
    return 0;
}
