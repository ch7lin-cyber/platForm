#include "HalGpio.h"

#include <stddef.h>
#include <string.h>

typedef struct
{
    const HalGpioInputDriverOps_t *ops;
    void *driver_context;
    bool registered;
    bool initialized;
} HalGpioInputInstance_t;

typedef struct
{
    const HalGpioOutputDriverOps_t *ops;
    void *driver_context;
    bool registered;
    bool initialized;
} HalGpioOutputInstance_t;

static HalGpioInputInstance_t
    g_inputs[HAL_GPIO_INPUT_CHANNEL_COUNT];
static HalGpioOutputInstance_t
    g_outputs[HAL_GPIO_OUTPUT_CHANNEL_COUNT];

HalGpioStatus_t HalGpio_RegisterInputDriver(
    uint8_t channel,
    const HalGpioInputDriverOps_t *ops,
    void *driver_context)
{
    HalGpioInputInstance_t *instance;

    if ((channel >= HAL_GPIO_INPUT_CHANNEL_COUNT) || (ops == NULL) ||
        (ops->initialize == NULL) || (ops->read == NULL))
    {
        return HAL_GPIO_STATUS_INVALID_ARGUMENT;
    }
    instance = &g_inputs[channel];
    (void)memset(instance, 0, sizeof(*instance));
    instance->ops = ops;
    instance->driver_context = driver_context;
    instance->registered = true;
    return HAL_GPIO_STATUS_OK;
}

HalGpioStatus_t HalGpio_RegisterOutputDriver(
    uint8_t channel,
    const HalGpioOutputDriverOps_t *ops,
    void *driver_context)
{
    HalGpioOutputInstance_t *instance;

    if ((channel >= HAL_GPIO_OUTPUT_CHANNEL_COUNT) || (ops == NULL) ||
        (ops->initialize == NULL) || (ops->write == NULL))
    {
        return HAL_GPIO_STATUS_INVALID_ARGUMENT;
    }
    instance = &g_outputs[channel];
    (void)memset(instance, 0, sizeof(*instance));
    instance->ops = ops;
    instance->driver_context = driver_context;
    instance->registered = true;
    return HAL_GPIO_STATUS_OK;
}

HalGpioStatus_t HalGpio_InitializeInput(uint8_t channel)
{
    HalGpioInputInstance_t *instance;
    HalGpioStatus_t status;

    if (channel >= HAL_GPIO_INPUT_CHANNEL_COUNT)
    {
        return HAL_GPIO_STATUS_INVALID_ARGUMENT;
    }
    instance = &g_inputs[channel];
    if (!instance->registered)
    {
        return HAL_GPIO_STATUS_NOT_REGISTERED;
    }
    status = instance->ops->initialize(instance->driver_context);
    if (status == HAL_GPIO_STATUS_OK)
    {
        instance->initialized = true;
    }
    return status;
}

HalGpioStatus_t HalGpio_InitializeOutput(uint8_t channel)
{
    HalGpioOutputInstance_t *instance;
    HalGpioStatus_t status;

    if (channel >= HAL_GPIO_OUTPUT_CHANNEL_COUNT)
    {
        return HAL_GPIO_STATUS_INVALID_ARGUMENT;
    }
    instance = &g_outputs[channel];
    if (!instance->registered)
    {
        return HAL_GPIO_STATUS_NOT_REGISTERED;
    }
    status = instance->ops->initialize(instance->driver_context);
    if (status == HAL_GPIO_STATUS_OK)
    {
        instance->initialized = true;
    }
    return status;
}

HalGpioStatus_t HalGpio_ReadInput(uint8_t channel, bool *active)
{
    HalGpioInputInstance_t *instance;

    if ((channel >= HAL_GPIO_INPUT_CHANNEL_COUNT) || (active == NULL))
    {
        return HAL_GPIO_STATUS_INVALID_ARGUMENT;
    }
    instance = &g_inputs[channel];
    if (!instance->registered)
    {
        return HAL_GPIO_STATUS_NOT_REGISTERED;
    }
    if (!instance->initialized)
    {
        return HAL_GPIO_STATUS_NOT_INITIALIZED;
    }
    return instance->ops->read(instance->driver_context, active);
}

HalGpioStatus_t HalGpio_WriteOutput(uint8_t channel, bool active)
{
    HalGpioOutputInstance_t *instance;

    if (channel >= HAL_GPIO_OUTPUT_CHANNEL_COUNT)
    {
        return HAL_GPIO_STATUS_INVALID_ARGUMENT;
    }
    instance = &g_outputs[channel];
    if (!instance->registered)
    {
        return HAL_GPIO_STATUS_NOT_REGISTERED;
    }
    if (!instance->initialized)
    {
        return HAL_GPIO_STATUS_NOT_INITIALIZED;
    }
    return instance->ops->write(instance->driver_context, active);
}

bool HalGpio_IsInputInitialized(uint8_t channel)
{
    return (channel < HAL_GPIO_INPUT_CHANNEL_COUNT) &&
           g_inputs[channel].registered && g_inputs[channel].initialized;
}

bool HalGpio_IsOutputInitialized(uint8_t channel)
{
    return (channel < HAL_GPIO_OUTPUT_CHANNEL_COUNT) &&
           g_outputs[channel].registered && g_outputs[channel].initialized;
}
