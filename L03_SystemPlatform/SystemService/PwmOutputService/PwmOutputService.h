#ifndef PWM_OUTPUT_SERVICE_H
#define PWM_OUTPUT_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

#include "HalPwm.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*PwmOutputInhibitProvider_t)(uint8_t channel, void *context);

typedef struct
{
    uint16_t requested_duty_permille;
    uint16_t applied_duty_permille;
    bool inhibited;
} PwmOutputState_t;

typedef enum
{
    PWM_OUTPUT_STATUS_OK = 0,
    PWM_OUTPUT_STATUS_INVALID_ARGUMENT,
    PWM_OUTPUT_STATUS_NOT_INITIALIZED,
    PWM_OUTPUT_STATUS_DRIVER_ERROR
} PwmOutputStatus_t;

PwmOutputStatus_t PwmOutputService_Initialize(
    uint8_t channel_count,
    PwmOutputInhibitProvider_t inhibit_provider,
    void *inhibit_context);
PwmOutputStatus_t PwmOutputService_SetCommand(
    uint8_t channel,
    uint16_t duty_permille);
PwmOutputStatus_t PwmOutputService_RefreshSafety(uint8_t channel);
bool PwmOutputService_GetState(uint8_t channel, PwmOutputState_t *state);

#ifdef __cplusplus
}
#endif

#endif
