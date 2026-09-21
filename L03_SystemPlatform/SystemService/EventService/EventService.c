#include "EventService.h"

#include <stddef.h>
#include <string.h>

typedef struct
{
    SerialConfigurationChangedEvent_t event;
    bool active;
} SerialEventSlot_t;

typedef struct
{
    TemperatureInputConfigurationChangedEvent_t event;
    bool active;
} TemperatureInputEventSlot_t;

static SerialEventSlot_t g_serial_events[EVENT_SERVICE_SERIAL_PORT_COUNT];
static TemperatureInputEventSlot_t
    g_temperature_input_events[EVENT_SERVICE_TEMPERATURE_INPUT_COUNT];
static uint32_t g_serial_required_ack_mask;
static uint32_t g_temperature_input_required_ack_mask;
static uint32_t g_next_event_id;
static bool g_event_service_initialized;

static void EnsureInitialized(void)
{
    if (!g_event_service_initialized)
    {
        (void)EventService_Initialize(EVENT_ACK_SERIAL_REQUIRED_DEFAULT);
    }
}

bool EventService_Initialize(uint32_t serial_required_ack_mask)
{
    if (serial_required_ack_mask == 0U)
    {
        return false;
    }

    (void)memset(g_serial_events, 0, sizeof(g_serial_events));
    (void)memset(g_temperature_input_events, 0,
                 sizeof(g_temperature_input_events));
    g_serial_required_ack_mask = serial_required_ack_mask;
    g_temperature_input_required_ack_mask =
        EVENT_ACK_TEMPERATURE_INPUT_REQUIRED_DEFAULT;
    g_next_event_id = 0U;
    g_event_service_initialized = true;
    return true;
}

bool EventService_ConfigureTemperatureInputRequiredAckMask(
    uint32_t required_ack_mask)
{
    uint8_t channel;
    const uint32_t supported_mask =
        EVENT_ACK_TEMPERATURE_INPUT_REQUIRED_DEFAULT;

    EnsureInitialized();
    if ((required_ack_mask == 0U) ||
        ((required_ack_mask & ~supported_mask) != 0U))
    {
        return false;
    }

    for (channel = 0U;
         channel < EVENT_SERVICE_TEMPERATURE_INPUT_COUNT;
         channel++)
    {
        if (g_temperature_input_events[channel].active)
        {
            return false;
        }
    }

    g_temperature_input_required_ack_mask = required_ack_mask;
    return true;
}

bool EventService_RaiseSerialConfigurationChanged(
    uint8_t port,
    uint16_t configuration_revision,
    const EventSerialConfiguration_t *old_configuration,
    const EventSerialConfiguration_t *new_configuration,
    uint32_t *event_id)
{
    SerialEventSlot_t *slot;

    EnsureInitialized();
    if ((port >= EVENT_SERVICE_SERIAL_PORT_COUNT) ||
        (old_configuration == NULL) ||
        (new_configuration == NULL))
    {
        return false;
    }

    g_next_event_id++;
    if (g_next_event_id == 0U)
    {
        g_next_event_id++;
    }

    slot = &g_serial_events[port];
    slot->event.event_id = g_next_event_id;
    slot->event.type = EVENT_TYPE_SERIAL_CONFIGURATION_CHANGED;
    slot->event.configuration_revision = configuration_revision;
    slot->event.port = port;
    slot->event.old_configuration = *old_configuration;
    slot->event.new_configuration = *new_configuration;
    slot->event.required_ack_mask = g_serial_required_ack_mask;
    slot->event.completed_ack_mask = 0U;
    slot->active = true;

    if (event_id != NULL)
    {
        *event_id = slot->event.event_id;
    }
    return true;
}

bool EventService_GetSerialConfigurationChanged(
    uint8_t port,
    SerialConfigurationChangedEvent_t *event)
{
    EnsureInitialized();
    if ((port >= EVENT_SERVICE_SERIAL_PORT_COUNT) ||
        (event == NULL) || (!g_serial_events[port].active))
    {
        return false;
    }

    *event = g_serial_events[port].event;
    return true;
}

bool EventService_RaiseTemperatureInputConfigurationChanged(
    uint8_t channel,
    uint16_t configuration_revision,
    uint32_t changed_mask,
    const EventTemperatureInputConfiguration_t *old_configuration,
    const EventTemperatureInputConfiguration_t *new_configuration,
    uint32_t *event_id)
{
    TemperatureInputEventSlot_t *slot;

    EnsureInitialized();
    if ((channel >= EVENT_SERVICE_TEMPERATURE_INPUT_COUNT) ||
        (configuration_revision == 0U) ||
        (changed_mask == 0U) ||
        ((changed_mask & ~EVENT_TEMPERATURE_INPUT_CHANGE_ALL) != 0U) ||
        (old_configuration == NULL) ||
        (new_configuration == NULL))
    {
        return false;
    }

    slot = &g_temperature_input_events[channel];
    if (slot->active)
    {
        /* Never overwrite an event that one or more consumers still need. */
        return false;
    }

    g_next_event_id++;
    if (g_next_event_id == 0U)
    {
        g_next_event_id++;
    }

    slot->event.event_id = g_next_event_id;
    slot->event.type =
        EVENT_TYPE_TEMPERATURE_INPUT_CONFIGURATION_CHANGED;
    slot->event.configuration_revision = configuration_revision;
    slot->event.channel = channel;
    slot->event.changed_mask = changed_mask;
    slot->event.old_configuration = *old_configuration;
    slot->event.new_configuration = *new_configuration;
    slot->event.required_ack_mask =
        g_temperature_input_required_ack_mask;
    slot->event.completed_ack_mask = 0U;
    slot->active = true;

    if (event_id != NULL)
    {
        *event_id = slot->event.event_id;
    }
    return true;
}

bool EventService_GetTemperatureInputConfigurationChanged(
    uint8_t channel,
    TemperatureInputConfigurationChangedEvent_t *event)
{
    EnsureInitialized();
    if ((channel >= EVENT_SERVICE_TEMPERATURE_INPUT_COUNT) ||
        (event == NULL) ||
        (!g_temperature_input_events[channel].active))
    {
        return false;
    }

    *event = g_temperature_input_events[channel].event;
    return true;
}

bool EventService_Acknowledge(uint32_t event_id, uint32_t consumer_mask)
{
    uint8_t port;
    uint8_t channel;

    EnsureInitialized();
    if ((event_id == 0U) || (consumer_mask == 0U))
    {
        return false;
    }

    for (port = 0U; port < EVENT_SERVICE_SERIAL_PORT_COUNT; port++)
    {
        SerialEventSlot_t *slot = &g_serial_events[port];

        if (slot->active && (slot->event.event_id == event_id))
        {
            if ((consumer_mask & ~slot->event.required_ack_mask) != 0U)
            {
                return false;
            }

            slot->event.completed_ack_mask |= consumer_mask;
            if ((slot->event.completed_ack_mask &
                 slot->event.required_ack_mask) ==
                slot->event.required_ack_mask)
            {
                slot->active = false;
            }
            return true;
        }
    }

    for (channel = 0U;
         channel < EVENT_SERVICE_TEMPERATURE_INPUT_COUNT;
         channel++)
    {
        TemperatureInputEventSlot_t *slot =
            &g_temperature_input_events[channel];

        if (slot->active && (slot->event.event_id == event_id))
        {
            if ((consumer_mask & ~slot->event.required_ack_mask) != 0U)
            {
                return false;
            }

            slot->event.completed_ack_mask |= consumer_mask;
            if ((slot->event.completed_ack_mask &
                 slot->event.required_ack_mask) ==
                slot->event.required_ack_mask)
            {
                slot->active = false;
            }
            return true;
        }
    }
    return false;
}

bool EventService_IsSerialConfigurationChangedPending(uint8_t port)
{
    EnsureInitialized();
    return ((port < EVENT_SERVICE_SERIAL_PORT_COUNT) &&
            g_serial_events[port].active);
}

bool EventService_IsTemperatureInputConfigurationChangedPending(
    uint8_t channel)
{
    EnsureInitialized();
    return ((channel < EVENT_SERVICE_TEMPERATURE_INPUT_COUNT) &&
            g_temperature_input_events[channel].active);
}
