#ifndef ALARM_CONFIGURATION_EVENT_CONSUMER_H
#define ALARM_CONFIGURATION_EVENT_CONSUMER_H

#include <stdbool.h>
#include <stdint.h>

#include "EventService.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*AlarmSensorRangeResolver_t)(
    const EventTemperatureInputConfiguration_t *configuration,
    float *minimum,
    float *maximum,
    bool *input_enabled,
    void *context);

typedef struct
{
    float setpoint_minimum;
    float setpoint_maximum;
    uint16_t configuration_revision;
    bool input_enabled;
    bool valid;
} AlarmConfigurationRange_t;

bool AlarmConfigurationEventConsumer_Initialize(
    AlarmSensorRangeResolver_t range_resolver,
    void *range_resolver_context);
bool AlarmConfigurationEventConsumer_Process(uint8_t channel);
bool AlarmConfigurationEventConsumer_GetRange(
    uint8_t channel,
    AlarmConfigurationRange_t *range);

#ifdef __cplusplus
}
#endif

#endif
