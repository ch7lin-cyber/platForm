#ifndef HAL_PWM_H
#define HAL_PWM_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HAL_PWM_CHANNEL_COUNT        (16U)
#define HAL_PWM_DUTY_MAX_PERMILLE    (1000U)

typedef enum
{
    /* Abort the active cycle and start a new cycle with the new period. */
    HAL_PWM_PERIOD_UPDATE_IMMEDIATE = 0,
    /* Preserve the active cycle and load the new period at its boundary. */
    HAL_PWM_PERIOD_UPDATE_NEXT_CYCLE
} HalPwmPeriodUpdateMode_t;

typedef enum
{
    HAL_PWM_STATUS_OK = 0,
    HAL_PWM_STATUS_INVALID_ARGUMENT,
    HAL_PWM_STATUS_NOT_REGISTERED,
    HAL_PWM_STATUS_NOT_INITIALIZED,
    HAL_PWM_STATUS_NOT_SUPPORTED,
    HAL_PWM_STATUS_IO_ERROR
} HalPwmStatus_t;

typedef struct
{
    HalPwmStatus_t (*initialize)(void *driver_context);
    HalPwmStatus_t (*set_duty_permille)(void *driver_context,
                                        uint16_t duty_permille);
    HalPwmStatus_t (*set_period_ms)(void *driver_context,
                                    uint32_t period_ms,
                                    HalPwmPeriodUpdateMode_t update_mode);
} HalPwmDriverOps_t;

HalPwmStatus_t HalPwm_RegisterDriver(uint8_t channel,
                                     const HalPwmDriverOps_t *ops,
                                     void *driver_context);
HalPwmStatus_t HalPwm_Initialize(uint8_t channel);
HalPwmStatus_t HalPwm_SetDutyPermille(uint8_t channel,
                                      uint16_t duty_permille);
HalPwmStatus_t HalPwm_SetPeriodMs(uint8_t channel,
                                 uint32_t period_ms,
                                 HalPwmPeriodUpdateMode_t update_mode);
bool HalPwm_IsInitialized(uint8_t channel);

#ifdef __cplusplus
}
#endif

#endif
