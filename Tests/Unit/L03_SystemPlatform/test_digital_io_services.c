#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "DigitalInputService.h"
#include "DigitalOutputService.h"
#include "HalGpio.h"

#define TEST_CHANNEL_COUNT (4U)

typedef struct
{
    bool state;
    bool fail_next;
} MockGpio_t;

static MockGpio_t g_inputs[TEST_CHANNEL_COUNT];
static MockGpio_t g_outputs[TEST_CHANNEL_COUNT];

static HalGpioStatus_t MockInitialize(void *context)
{
    return (context != NULL) ? HAL_GPIO_STATUS_OK :
                               HAL_GPIO_STATUS_INVALID_ARGUMENT;
}

static HalGpioStatus_t MockRead(void *context, bool *active)
{
    MockGpio_t *gpio = (MockGpio_t *)context;
    if ((gpio == NULL) || (active == NULL))
    {
        return HAL_GPIO_STATUS_INVALID_ARGUMENT;
    }
    if (gpio->fail_next)
    {
        gpio->fail_next = false;
        return HAL_GPIO_STATUS_IO_ERROR;
    }
    *active = gpio->state;
    return HAL_GPIO_STATUS_OK;
}

static HalGpioStatus_t MockWrite(void *context, bool active)
{
    MockGpio_t *gpio = (MockGpio_t *)context;
    if (gpio == NULL)
    {
        return HAL_GPIO_STATUS_INVALID_ARGUMENT;
    }
    if (gpio->fail_next)
    {
        gpio->fail_next = false;
        return HAL_GPIO_STATUS_IO_ERROR;
    }
    gpio->state = active;
    return HAL_GPIO_STATUS_OK;
}

int main(void)
{
    static const HalGpioInputDriverOps_t input_ops =
        {MockInitialize, MockRead};
    static const HalGpioOutputDriverOps_t output_ops =
        {MockInitialize, MockWrite};
    DigitalInputSnapshot_t input_snapshot;
    DigitalOutputState_t output_state;
    uint8_t channel;

    for (channel = 0U; channel < TEST_CHANNEL_COUNT; channel++)
    {
        g_inputs[channel].state = (channel == 0U) || (channel == 2U);
        assert(HalGpio_RegisterInputDriver(
                   channel, &input_ops, &g_inputs[channel]) ==
               HAL_GPIO_STATUS_OK);
        assert(HalGpio_RegisterOutputDriver(
                   channel, &output_ops, &g_outputs[channel]) ==
               HAL_GPIO_STATUS_OK);
    }

    assert(DigitalInputService_Initialize(TEST_CHANNEL_COUNT) ==
           DIGITAL_INPUT_STATUS_OK);
    assert(DigitalInputService_GetSnapshot(&input_snapshot));
    assert(input_snapshot.state_mask == 0x0005U);
    assert(input_snapshot.changed_mask == 0U);
    assert(input_snapshot.revision == 0U);

    for (channel = 0U; channel < TEST_CHANNEL_COUNT; channel++)
    {
        g_inputs[channel].state = (channel == 1U) || (channel == 3U);
    }
    assert(DigitalInputService_Process() == DIGITAL_INPUT_STATUS_OK);
    assert(DigitalInputService_GetSnapshot(&input_snapshot));
    assert(input_snapshot.state_mask == 0x000AU);
    assert(input_snapshot.changed_mask == 0x000FU);
    assert(input_snapshot.revision == 1U);
    DigitalInputService_ClearChangedMask(0x0003U);
    assert(DigitalInputService_GetSnapshot(&input_snapshot));
    assert(input_snapshot.changed_mask == 0x000CU);

    g_inputs[2].fail_next = true;
    assert(DigitalInputService_Process() ==
           DIGITAL_INPUT_STATUS_DRIVER_ERROR);
    assert(DigitalInputService_GetSnapshot(&input_snapshot));
    assert(input_snapshot.state_mask == 0x000AU);

    assert(DigitalOutputService_Initialize(TEST_CHANNEL_COUNT) ==
           DIGITAL_OUTPUT_STATUS_OK);
    assert(DigitalOutputService_SetMask(0x0005U) ==
           DIGITAL_OUTPUT_STATUS_OK);
    assert(g_outputs[0].state && !g_outputs[1].state &&
           g_outputs[2].state && !g_outputs[3].state);
    assert(DigitalOutputService_GetState(&output_state));
    assert(output_state.active_mask == 0x0005U);
    assert(output_state.revision == 1U);

    g_outputs[2].fail_next = true;
    assert(DigitalOutputService_SetMask(0x000AU) ==
           DIGITAL_OUTPUT_STATUS_DRIVER_ERROR);
    assert(DigitalOutputService_GetState(&output_state));
    assert(output_state.active_mask == 0x0005U);
    assert(output_state.failed_channel == 2U);
    assert(g_outputs[0].state && !g_outputs[1].state &&
           g_outputs[2].state && !g_outputs[3].state);

    assert(DigitalOutputService_SetChannel(3U, true) ==
           DIGITAL_OUTPUT_STATUS_OK);
    assert(DigitalOutputService_GetState(&output_state));
    assert(output_state.active_mask == 0x000DU);
    assert(output_state.revision == 2U);
    assert(output_state.failed_channel == DIGITAL_OUTPUT_FAILED_CHANNEL_NONE);
    return 0;
}
