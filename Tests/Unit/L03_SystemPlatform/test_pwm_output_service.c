#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "HalPwm.h"
#include "PwmOutputService.h"

typedef struct
{
    uint16_t duty_permille;
    uint32_t period_ms;
    HalPwmPeriodUpdateMode_t update_mode;
    uint32_t initialize_count;
    uint32_t set_count;
} MockPwmDriver_t;

static bool g_inhibited;

static HalPwmStatus_t MockInitialize(void *context)
{
    MockPwmDriver_t *driver = (MockPwmDriver_t *)context;
    driver->initialize_count++;
    driver->duty_permille = 0U;
    return HAL_PWM_STATUS_OK;
}

static HalPwmStatus_t MockSetDuty(void *context, uint16_t duty_permille)
{
    MockPwmDriver_t *driver = (MockPwmDriver_t *)context;
    driver->set_count++;
    driver->duty_permille = duty_permille;
    return HAL_PWM_STATUS_OK;
}

static HalPwmStatus_t MockSetPeriod(
    void *context,
    uint32_t period_ms,
    HalPwmPeriodUpdateMode_t update_mode)
{
    MockPwmDriver_t *driver = (MockPwmDriver_t *)context;
    driver->period_ms = period_ms;
    driver->update_mode = update_mode;
    return HAL_PWM_STATUS_OK;
}

static bool IsInhibited(uint8_t channel, void *context)
{
    (void)channel;
    (void)context;
    return g_inhibited;
}

int main(void)
{
    static const HalPwmDriverOps_t ops =
        {MockInitialize, MockSetDuty, MockSetPeriod};
    MockPwmDriver_t driver = {0U};
    PwmOutputState_t state;

    assert(HalPwm_RegisterDriver(0U, &ops, &driver) == HAL_PWM_STATUS_OK);
    g_inhibited = true;
    assert(PwmOutputService_Initialize(1U, IsInhibited, NULL) ==
           PWM_OUTPUT_STATUS_OK);
    assert(driver.initialize_count == 1U);
    assert(driver.duty_permille == 0U);
    assert(driver.period_ms == PWM_OUTPUT_PERIOD_DEFAULT_MS);

    assert(PwmOutputService_SetPeriod(
               0U, 250U, PWM_OUTPUT_PERIOD_UPDATE_NEXT_CYCLE) ==
           PWM_OUTPUT_STATUS_OK);
    assert(driver.period_ms == 250U);
    assert(driver.update_mode == HAL_PWM_PERIOD_UPDATE_NEXT_CYCLE);
    assert(PwmOutputService_GetState(0U, &state));
    assert(state.requested_period_ms == 250U);
    assert(state.period_update_mode == PWM_OUTPUT_PERIOD_UPDATE_NEXT_CYCLE);

    assert(PwmOutputService_SetPeriod(
               0U, 1000U, PWM_OUTPUT_PERIOD_UPDATE_IMMEDIATE) ==
           PWM_OUTPUT_STATUS_OK);
    assert(driver.period_ms == 1000U);
    assert(driver.update_mode == HAL_PWM_PERIOD_UPDATE_IMMEDIATE);
    assert(PwmOutputService_SetPeriod(
               0U, PWM_OUTPUT_PERIOD_MIN_MS - 1U,
               PWM_OUTPUT_PERIOD_UPDATE_IMMEDIATE) ==
           PWM_OUTPUT_STATUS_INVALID_ARGUMENT);
    assert(PwmOutputService_SetPeriod(
               0U, PWM_OUTPUT_PERIOD_MAX_MS + 1U,
               PWM_OUTPUT_PERIOD_UPDATE_IMMEDIATE) ==
           PWM_OUTPUT_STATUS_INVALID_ARGUMENT);

    assert(PwmOutputService_SetCommand(0U, 700U) == PWM_OUTPUT_STATUS_OK);
    assert(driver.duty_permille == 0U);
    assert(PwmOutputService_GetState(0U, &state));
    assert(state.requested_duty_permille == 700U);
    assert(state.applied_duty_permille == 0U);
    assert(state.inhibited);

    g_inhibited = false;
    assert(PwmOutputService_RefreshSafety(0U) == PWM_OUTPUT_STATUS_OK);
    assert(driver.duty_permille == 700U);
    assert(PwmOutputService_GetState(0U, &state));
    assert(state.applied_duty_permille == 700U);
    assert(!state.inhibited);

    g_inhibited = true;
    assert(PwmOutputService_RefreshSafety(0U) == PWM_OUTPUT_STATUS_OK);
    assert(driver.duty_permille == 0U);
    assert(PwmOutputService_SetCommand(0U, 1001U) ==
           PWM_OUTPUT_STATUS_INVALID_ARGUMENT);
    return 0;
}
