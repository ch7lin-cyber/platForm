#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "HalPwm.h"
#include "PwmOutputService.h"

typedef struct
{
    uint16_t duty_permille;
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

static bool IsInhibited(uint8_t channel, void *context)
{
    (void)channel;
    (void)context;
    return g_inhibited;
}

int main(void)
{
    static const HalPwmDriverOps_t ops =
        {MockInitialize, MockSetDuty};
    MockPwmDriver_t driver = {0U, 0U, 0U};
    PwmOutputState_t state;

    assert(HalPwm_RegisterDriver(0U, &ops, &driver) == HAL_PWM_STATUS_OK);
    g_inhibited = true;
    assert(PwmOutputService_Initialize(1U, IsInhibited, NULL) ==
           PWM_OUTPUT_STATUS_OK);
    assert(driver.initialize_count == 1U);
    assert(driver.duty_permille == 0U);

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
