#ifndef EVENT_SERVICE_H
#define EVENT_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

#include "SerialConfiguration.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EVENT_SERVICE_SERIAL_PORT_COUNT       (2U)
#define EVENT_SERVICE_TEMPERATURE_INPUT_COUNT (16U)

#define EVENT_ACK_COMMUNICATION               (1UL << 0)
#define EVENT_ACK_NVM                         (1UL << 1)
#define EVENT_ACK_HMI                         (1UL << 2)
#define EVENT_ACK_DIAGNOSTICS                 (1UL << 3)
#define EVENT_ACK_ALARM                       (1UL << 4)
#define EVENT_ACK_CONTROL                     (1UL << 5)
#define EVENT_ACK_SAFETY                      (1UL << 6)
#define EVENT_ACK_SERIAL_REQUIRED_DEFAULT     \
    (EVENT_ACK_COMMUNICATION | EVENT_ACK_NVM | \
     EVENT_ACK_HMI | EVENT_ACK_DIAGNOSTICS)
#define EVENT_ACK_TEMPERATURE_INPUT_REQUIRED_DEFAULT \
    (EVENT_ACK_NVM | EVENT_ACK_HMI | EVENT_ACK_ALARM | \
     EVENT_ACK_CONTROL | EVENT_ACK_SAFETY)

#define EVENT_TEMPERATURE_INPUT_CHANGE_FILTER_TIME_CONSTANT (1UL << 0)
#define EVENT_TEMPERATURE_INPUT_CHANGE_SENSOR_TYPE          (1UL << 1)
#define EVENT_TEMPERATURE_INPUT_CHANGE_TC_LINEARIZATION     (1UL << 2)
#define EVENT_TEMPERATURE_INPUT_CHANGE_ALL                  \
    (EVENT_TEMPERATURE_INPUT_CHANGE_FILTER_TIME_CONSTANT |  \
     EVENT_TEMPERATURE_INPUT_CHANGE_SENSOR_TYPE |           \
     EVENT_TEMPERATURE_INPUT_CHANGE_TC_LINEARIZATION)

typedef enum
{
    EVENT_TYPE_NONE = 0,
    EVENT_TYPE_SERIAL_CONFIGURATION_CHANGED,
    EVENT_TYPE_TEMPERATURE_INPUT_CONFIGURATION_CHANGED
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

typedef struct
{
    float filter_time_constant_seconds;
    uint16_t sensor_type;
    uint16_t tc_linearization;
} EventTemperatureInputConfiguration_t;

typedef struct
{
    uint32_t event_id;
    EventType_t type;
    uint16_t configuration_revision;
    uint8_t channel;
    uint32_t changed_mask;
    EventTemperatureInputConfiguration_t old_configuration;
    EventTemperatureInputConfiguration_t new_configuration;
    uint32_t required_ack_mask;
    uint32_t completed_ack_mask;
} TemperatureInputConfigurationChangedEvent_t;

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
bool EventService_RaiseTemperatureInputConfigurationChanged(
    uint8_t channel,
    uint16_t configuration_revision,
    uint32_t changed_mask,
    const EventTemperatureInputConfiguration_t *old_configuration,
    const EventTemperatureInputConfiguration_t *new_configuration,
    uint32_t *event_id);
bool EventService_GetTemperatureInputConfigurationChanged(
    uint8_t channel,
    TemperatureInputConfigurationChangedEvent_t *event);
bool EventService_Acknowledge(uint32_t event_id, uint32_t consumer_mask);
bool EventService_IsSerialConfigurationChangedPending(uint8_t port);
bool EventService_IsTemperatureInputConfigurationChangedPending(
    uint8_t channel);

#ifdef __cplusplus
}
#endif

#endif
