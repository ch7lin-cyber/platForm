#include "EventService.h"

#include <stddef.h>
#include <string.h>

typedef struct
{
    SerialConfigurationChangedEvent_t event;
    bool active;
} SerialEventSlot_t;

static SerialEventSlot_t g_serial_events[EVENT_SERVICE_SERIAL_PORT_COUNT];
static uint32_t g_serial_required_ack_mask;
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
    g_serial_required_ack_mask = serial_required_ack_mask;
    g_next_event_id = 0U;
    g_event_service_initialized = true;
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

bool EventService_Acknowledge(uint32_t event_id, uint32_t consumer_mask)
{
    uint8_t port;

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
    return false;
}

bool EventService_IsSerialConfigurationChangedPending(uint8_t port)
{
    EnsureInitialized();
    return ((port < EVENT_SERVICE_SERIAL_PORT_COUNT) &&
            g_serial_events[port].active);
}
