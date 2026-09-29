#ifndef PWM_OUTPUT_SERVICE_H
#define PWM_OUTPUT_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

#include "HalPwm.h"

#ifdef __cplusplus
extern "C" {
#endif

/* PWM period contract shared by all platform consumers. */
#define PWM_OUTPUT_PERIOD_MIN_MS     (10UL)
#define PWM_OUTPUT_PERIOD_MAX_MS     (10000UL)
#define PWM_OUTPUT_PERIOD_DEFAULT_MS (10UL)

typedef enum
{
    /* Abort the active cycle and start a new cycle with the new period. */
    PWM_OUTPUT_PERIOD_UPDATE_IMMEDIATE = 0,
    /* Preserve the active cycle and apply the new period at its boundary. */
    PWM_OUTPUT_PERIOD_UPDATE_NEXT_CYCLE
} PwmOutputPeriodUpdateMode_t;

typedef bool (*PwmOutputInhibitProvider_t)(uint8_t channel, void *context);

typedef struct
{
    uint16_t requested_duty_permille;
    uint16_t applied_duty_permille;
    uint32_t requested_period_ms;
    PwmOutputPeriodUpdateMode_t period_update_mode;
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
PwmOutputStatus_t PwmOutputService_SetPeriod(
    uint8_t channel,
    uint32_t period_ms,
    PwmOutputPeriodUpdateMode_t update_mode);
PwmOutputStatus_t PwmOutputService_RefreshSafety(uint8_t channel);
bool PwmOutputService_GetState(uint8_t channel, PwmOutputState_t *state);

#ifdef __cplusplus
}
#endif

#endif
