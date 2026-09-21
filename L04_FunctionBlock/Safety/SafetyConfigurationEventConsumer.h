#ifndef SAFETY_CONFIGURATION_EVENT_CONSUMER_H
#define SAFETY_CONFIGURATION_EVENT_CONSUMER_H

#include <stdbool.h>
#include <stdint.h>

#include "EventService.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*SafetySensorRangeResolver_t)(
    const EventTemperatureInputConfiguration_t *configuration,
    float *minimum,
    float *maximum,
    bool *input_enabled,
    void *context);

typedef struct
{
    float measurement_minimum;
    float measurement_maximum;
    uint16_t configuration_revision;
    bool input_enabled;
    bool output_inhibit;
    bool valid;
} SafetyConfigurationRange_t;

bool SafetyConfigurationEventConsumer_Initialize(
    SafetySensorRangeResolver_t range_resolver,
    void *range_resolver_context);
bool SafetyConfigurationEventConsumer_Process(uint8_t channel);
bool SafetyConfigurationEventConsumer_GetRange(
    uint8_t channel,
    SafetyConfigurationRange_t *range);
bool SafetyConfigurationEventConsumer_IsOutputInhibited(uint8_t channel);

#ifdef __cplusplus
}
#endif

#endif
