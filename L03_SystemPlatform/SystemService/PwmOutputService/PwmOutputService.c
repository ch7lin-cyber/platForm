#include "PwmOutputService.h"

#include <stddef.h>
#include <string.h>

static PwmOutputState_t g_pwm_output_state[HAL_PWM_CHANNEL_COUNT];
static PwmOutputInhibitProvider_t g_inhibit_provider;
static void *g_inhibit_context;
static uint8_t g_channel_count;
static bool g_initialized;

static PwmOutputStatus_t ApplyChannel(uint8_t channel)
{
    PwmOutputState_t new_state = g_pwm_output_state[channel];
    HalPwmStatus_t hal_status;

    new_state.inhibited =
        g_inhibit_provider(channel, g_inhibit_context);
    new_state.applied_duty_permille =
        new_state.inhibited ? 0U : new_state.requested_duty_permille;
    hal_status = HalPwm_SetDutyPermille(
        channel, new_state.applied_duty_permille);
    if (hal_status != HAL_PWM_STATUS_OK)
    {
        return PWM_OUTPUT_STATUS_DRIVER_ERROR;
    }

    g_pwm_output_state[channel] = new_state;
    return PWM_OUTPUT_STATUS_OK;
}

PwmOutputStatus_t PwmOutputService_Initialize(
    uint8_t channel_count,
    PwmOutputInhibitProvider_t inhibit_provider,
    void *inhibit_context)
{
    uint8_t channel;

    if ((channel_count == 0U) || (channel_count > HAL_PWM_CHANNEL_COUNT) ||
        (inhibit_provider == NULL))
    {
        return PWM_OUTPUT_STATUS_INVALID_ARGUMENT;
    }

    (void)memset(g_pwm_output_state, 0, sizeof(g_pwm_output_state));
    g_inhibit_provider = inhibit_provider;
    g_inhibit_context = inhibit_context;
    g_channel_count = channel_count;
    for (channel = 0U; channel < channel_count; channel++)
    {
        if (HalPwm_Initialize(channel) != HAL_PWM_STATUS_OK)
        {
            g_initialized = false;
            return PWM_OUTPUT_STATUS_DRIVER_ERROR;
        }
        if (HalPwm_SetDutyPermille(channel, 0U) != HAL_PWM_STATUS_OK)
        {
            g_initialized = false;
            return PWM_OUTPUT_STATUS_DRIVER_ERROR;
        }
        if (HalPwm_SetPeriodMs(
                channel, PWM_OUTPUT_PERIOD_DEFAULT_MS,
                HAL_PWM_PERIOD_UPDATE_IMMEDIATE) != HAL_PWM_STATUS_OK)
        {
            g_initialized = false;
            return PWM_OUTPUT_STATUS_DRIVER_ERROR;
        }
        g_pwm_output_state[channel].requested_period_ms =
            PWM_OUTPUT_PERIOD_DEFAULT_MS;
        g_pwm_output_state[channel].period_update_mode =
            PWM_OUTPUT_PERIOD_UPDATE_IMMEDIATE;
        g_pwm_output_state[channel].inhibited = true;
    }
    g_initialized = true;
    return PWM_OUTPUT_STATUS_OK;
}

PwmOutputStatus_t PwmOutputService_SetPeriod(
    uint8_t channel,
    uint32_t period_ms,
    PwmOutputPeriodUpdateMode_t update_mode)
{
    HalPwmPeriodUpdateMode_t hal_update_mode;

    if ((channel >= g_channel_count) ||
        (period_ms < PWM_OUTPUT_PERIOD_MIN_MS) ||
        (period_ms > PWM_OUTPUT_PERIOD_MAX_MS) ||
        ((update_mode != PWM_OUTPUT_PERIOD_UPDATE_IMMEDIATE) &&
         (update_mode != PWM_OUTPUT_PERIOD_UPDATE_NEXT_CYCLE)))
    {
        return PWM_OUTPUT_STATUS_INVALID_ARGUMENT;
    }
    if (!g_initialized)
    {
        return PWM_OUTPUT_STATUS_NOT_INITIALIZED;
    }

    hal_update_mode =
        (update_mode == PWM_OUTPUT_PERIOD_UPDATE_IMMEDIATE) ?
        HAL_PWM_PERIOD_UPDATE_IMMEDIATE :
        HAL_PWM_PERIOD_UPDATE_NEXT_CYCLE;
    if (HalPwm_SetPeriodMs(channel, period_ms, hal_update_mode) !=
        HAL_PWM_STATUS_OK)
    {
        return PWM_OUTPUT_STATUS_DRIVER_ERROR;
    }

    g_pwm_output_state[channel].requested_period_ms = period_ms;
    g_pwm_output_state[channel].period_update_mode = update_mode;
    return PWM_OUTPUT_STATUS_OK;
}

PwmOutputStatus_t PwmOutputService_SetCommand(
    uint8_t channel,
    uint16_t duty_permille)
{
    if ((channel >= g_channel_count) ||
        (duty_permille > HAL_PWM_DUTY_MAX_PERMILLE))
    {
        return PWM_OUTPUT_STATUS_INVALID_ARGUMENT;
    }
    if (!g_initialized)
    {
        return PWM_OUTPUT_STATUS_NOT_INITIALIZED;
    }

    g_pwm_output_state[channel].requested_duty_permille = duty_permille;
    return ApplyChannel(channel);
}

PwmOutputStatus_t PwmOutputService_RefreshSafety(uint8_t channel)
{
    if (channel >= g_channel_count)
    {
        return PWM_OUTPUT_STATUS_INVALID_ARGUMENT;
    }
    if (!g_initialized)
    {
        return PWM_OUTPUT_STATUS_NOT_INITIALIZED;
    }
    return ApplyChannel(channel);
}

bool PwmOutputService_GetState(uint8_t channel, PwmOutputState_t *state)
{
    if ((!g_initialized) || (channel >= g_channel_count) || (state == NULL))
    {
        return false;
    }
    *state = g_pwm_output_state[channel];
    return true;
}
