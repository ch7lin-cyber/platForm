#include <assert.h>
#include <stdint.h>

#include "EventService.h"

static void TestTemperatureInputEventLifecycle(void)
{
    EventTemperatureInputConfiguration_t old_configuration =
        {0.5F, 62U, 46U};
    EventTemperatureInputConfiguration_t new_configuration =
        {2.0F, 95U, 48U};
    TemperatureInputConfigurationChangedEvent_t event;
    uint32_t event_id;

    assert(EventService_Initialize(EVENT_ACK_SERIAL_REQUIRED_DEFAULT));
    assert(!EventService_ConfigureTemperatureInputRequiredAckMask(0U));
    assert(!EventService_ConfigureTemperatureInputRequiredAckMask(
        EVENT_ACK_DIAGNOSTICS));
    assert(EventService_RaiseTemperatureInputConfigurationChanged(
        0U, 1U, EVENT_TEMPERATURE_INPUT_CHANGE_ALL,
        &old_configuration, &new_configuration, &event_id));
    assert(event_id != 0U);
    assert(EventService_IsTemperatureInputConfigurationChangedPending(0U));
    assert(EventService_GetTemperatureInputConfigurationChanged(0U, &event));
    assert(event.event_id == event_id);
    assert(event.type ==
           EVENT_TYPE_TEMPERATURE_INPUT_CONFIGURATION_CHANGED);
    assert(event.configuration_revision == 1U);
    assert(event.channel == 0U);
    assert(event.changed_mask == EVENT_TEMPERATURE_INPUT_CHANGE_ALL);
    assert(event.old_configuration.sensor_type == 62U);
    assert(event.new_configuration.sensor_type == 95U);

    /* An unacknowledged event may not be overwritten. */
    assert(!EventService_RaiseTemperatureInputConfigurationChanged(
        0U, 2U, EVENT_TEMPERATURE_INPUT_CHANGE_SENSOR_TYPE,
        &new_configuration, &old_configuration, NULL));
    assert(!EventService_ConfigureTemperatureInputRequiredAckMask(
        EVENT_ACK_ALARM));
    assert(!EventService_Acknowledge(event_id, EVENT_ACK_DIAGNOSTICS));

    assert(EventService_Acknowledge(event_id, EVENT_ACK_ALARM));
    assert(EventService_IsTemperatureInputConfigurationChangedPending(0U));
    assert(EventService_Acknowledge(event_id, EVENT_ACK_CONTROL));
    assert(EventService_Acknowledge(event_id, EVENT_ACK_SAFETY));
    assert(EventService_Acknowledge(event_id, EVENT_ACK_HMI));
    assert(EventService_IsTemperatureInputConfigurationChangedPending(0U));
    assert(EventService_Acknowledge(event_id, EVENT_ACK_NVM));
    assert(!EventService_IsTemperatureInputConfigurationChangedPending(0U));
    assert(!EventService_GetTemperatureInputConfigurationChanged(0U, &event));
    assert(EventService_ConfigureTemperatureInputRequiredAckMask(
        EVENT_ACK_ALARM));
}

static void TestTemperatureInputEventValidation(void)
{
    EventTemperatureInputConfiguration_t configuration =
        {0.5F, 62U, 46U};

    assert(!EventService_RaiseTemperatureInputConfigurationChanged(
        EVENT_SERVICE_TEMPERATURE_INPUT_COUNT, 1U,
        EVENT_TEMPERATURE_INPUT_CHANGE_SENSOR_TYPE,
        &configuration, &configuration, NULL));
    assert(!EventService_RaiseTemperatureInputConfigurationChanged(
        0U, 0U, EVENT_TEMPERATURE_INPUT_CHANGE_SENSOR_TYPE,
        &configuration, &configuration, NULL));
    assert(!EventService_RaiseTemperatureInputConfigurationChanged(
        0U, 1U, 0U, &configuration, &configuration, NULL));
}

int main(void)
{
    TestTemperatureInputEventLifecycle();
    TestTemperatureInputEventValidation();
    return 0;
}
