#ifndef EVENT_SERVICE_H
#define EVENT_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

#include "SerialConfiguration.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EVENT_SERVICE_SERIAL_PORT_COUNT       (2U)

#define EVENT_ACK_COMMUNICATION               (1UL << 0)
#define EVENT_ACK_NVM                         (1UL << 1)
#define EVENT_ACK_HMI                         (1UL << 2)
#define EVENT_ACK_DIAGNOSTICS                 (1UL << 3)
#define EVENT_ACK_SERIAL_REQUIRED_DEFAULT     \
    (EVENT_ACK_COMMUNICATION | EVENT_ACK_NVM | \
     EVENT_ACK_HMI | EVENT_ACK_DIAGNOSTICS)

typedef enum
{
    EVENT_TYPE_NONE = 0,
    EVENT_TYPE_SERIAL_CONFIGURATION_CHANGED
} EventType_t;

typedef struct
{
    SerialConfiguration_t serial;
    uint16_t unit_id;
} EventSerialConfiguration_t;

typedef struct
{
    uint32_t event_id;
    EventType_t type;
    uint16_t configuration_revision;
    uint8_t port;
    EventSerialConfiguration_t old_configuration;
    EventSerialConfiguration_t new_configuration;
    uint32_t required_ack_mask;
    uint32_t completed_ack_mask;
} SerialConfigurationChangedEvent_t;

bool EventService_Initialize(uint32_t serial_required_ack_mask);
bool EventService_RaiseSerialConfigurationChanged(
    uint8_t port,
    uint16_t configuration_revision,
    const EventSerialConfiguration_t *old_configuration,
    const EventSerialConfiguration_t *new_configuration,
    uint32_t *event_id);
bool EventService_GetSerialConfigurationChanged(
    uint8_t port,
    SerialConfigurationChangedEvent_t *event);
bool EventService_Acknowledge(uint32_t event_id, uint32_t consumer_mask);
bool EventService_IsSerialConfigurationChangedPending(uint8_t port);

#ifdef __cplusplus
}
#endif

#endif
