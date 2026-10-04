#ifndef DIGITAL_INPUT_SERVICE_H
#define DIGITAL_INPUT_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    DIGITAL_INPUT_STATUS_OK = 0,
    DIGITAL_INPUT_STATUS_INVALID_ARGUMENT,
    DIGITAL_INPUT_STATUS_NOT_INITIALIZED,
    DIGITAL_INPUT_STATUS_DRIVER_ERROR
} DigitalInputStatus_t;

typedef struct
{
    uint16_t state_mask;
    uint16_t changed_mask;
    uint16_t valid_mask;
    uint16_t revision;
} DigitalInputSnapshot_t;

DigitalInputStatus_t DigitalInputService_Initialize(uint8_t channel_count);
DigitalInputStatus_t DigitalInputService_Process(void);
bool DigitalInputService_GetState(uint8_t channel, bool *active);
bool DigitalInputService_GetSnapshot(DigitalInputSnapshot_t *snapshot);
void DigitalInputService_ClearChangedMask(uint16_t mask);

#ifdef __cplusplus
}
#endif

#endif
