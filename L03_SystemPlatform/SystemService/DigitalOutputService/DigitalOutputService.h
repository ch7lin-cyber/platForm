#ifndef DIGITAL_OUTPUT_SERVICE_H
#define DIGITAL_OUTPUT_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    DIGITAL_OUTPUT_STATUS_OK = 0,
    DIGITAL_OUTPUT_STATUS_INVALID_ARGUMENT,
    DIGITAL_OUTPUT_STATUS_NOT_INITIALIZED,
    DIGITAL_OUTPUT_STATUS_DRIVER_ERROR,
    DIGITAL_OUTPUT_STATUS_ROLLBACK_ERROR
} DigitalOutputStatus_t;

typedef struct
{
    uint16_t active_mask;
    uint16_t valid_mask;
    uint16_t revision;
    DigitalOutputStatus_t last_status;
    uint16_t failed_channel;
} DigitalOutputState_t;

#define DIGITAL_OUTPUT_FAILED_CHANNEL_NONE (0xFFFFU)

DigitalOutputStatus_t DigitalOutputService_Initialize(uint8_t channel_count);
DigitalOutputStatus_t DigitalOutputService_SetChannel(uint8_t channel,
                                                       bool active);
DigitalOutputStatus_t DigitalOutputService_SetMask(uint16_t active_mask);
bool DigitalOutputService_GetState(DigitalOutputState_t *state);

#ifdef __cplusplus
}
#endif

#endif
