#include "HalPwm.h"

#include <stddef.h>
#include <string.h>

typedef struct
{
    const HalPwmDriverOps_t *ops;
    void *driver_context;
    bool registered;
    bool initialized;
} HalPwmInstance_t;

static HalPwmInstance_t g_hal_pwm[HAL_PWM_CHANNEL_COUNT];

HalPwmStatus_t HalPwm_RegisterDriver(uint8_t channel,
                                     const HalPwmDriverOps_t *ops,
                                     void *driver_context)
{
    HalPwmInstance_t *instance;

    if ((channel >= HAL_PWM_CHANNEL_COUNT) || (ops == NULL) ||
        (ops->initialize == NULL) || (ops->set_duty_permille == NULL))
    {
        return HAL_PWM_STATUS_INVALID_ARGUMENT;
    }

    instance = &g_hal_pwm[channel];
    (void)memset(instance, 0, sizeof(*instance));
    instance->ops = ops;
    instance->driver_context = driver_context;
    instance->registered = true;
    return HAL_PWM_STATUS_OK;
}

HalPwmStatus_t HalPwm_Initialize(uint8_t channel)
{
    HalPwmInstance_t *instance;
    HalPwmStatus_t status;

    if (channel >= HAL_PWM_CHANNEL_COUNT)
    {
        return HAL_PWM_STATUS_INVALID_ARGUMENT;
    }
    instance = &g_hal_pwm[channel];
    if (!instance->registered)
    {
        return HAL_PWM_STATUS_NOT_REGISTERED;
    }

    status = instance->ops->initialize(instance->driver_context);
    if (status == HAL_PWM_STATUS_OK)
    {
        instance->initialized = true;
    }
    return status;
}

HalPwmStatus_t HalPwm_SetDutyPermille(uint8_t channel,
                                      uint16_t duty_permille)
{
    HalPwmInstance_t *instance;

    if ((channel >= HAL_PWM_CHANNEL_COUNT) ||
        (duty_permille > HAL_PWM_DUTY_MAX_PERMILLE))
    {
        return HAL_PWM_STATUS_INVALID_ARGUMENT;
    }
    instance = &g_hal_pwm[channel];
    if (!instance->registered)
    {
        return HAL_PWM_STATUS_NOT_REGISTERED;
    }
    if (!instance->initialized)
    {
        return HAL_PWM_STATUS_NOT_INITIALIZED;
    }
    return instance->ops->set_duty_permille(instance->driver_context,
                                             duty_permille);
}

bool HalPwm_IsInitialized(uint8_t channel)
{
    return (channel < HAL_PWM_CHANNEL_COUNT) &&
           g_hal_pwm[channel].registered &&
           g_hal_pwm[channel].initialized;
}
