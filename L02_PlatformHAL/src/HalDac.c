#include "HalDac.h"

#include <stddef.h>
#include <string.h>

typedef struct
{
    const HalDacDriverOps_t *ops;
    void *driver_context;
    uint16_t last_code;
    bool registered;
    bool initialized;
} HalDacInstance_t;

static HalDacInstance_t g_hal_dac[HAL_DAC_CHANNEL_COUNT];

HalDacStatus_t HalDac_RegisterDriver(uint8_t channel,
                                    const HalDacDriverOps_t *ops,
                                    void *driver_context)
{
    HalDacInstance_t *instance;

    if ((channel >= HAL_DAC_CHANNEL_COUNT) || (ops == NULL) ||
        (ops->initialize == NULL) || (ops->write_code == NULL))
    {
        return HAL_DAC_STATUS_INVALID_ARGUMENT;
    }

    instance = &g_hal_dac[channel];
    (void)memset(instance, 0, sizeof(*instance));
    instance->ops = ops;
    instance->driver_context = driver_context;
    instance->registered = true;
    return HAL_DAC_STATUS_OK;
}

HalDacStatus_t HalDac_Initialize(uint8_t channel)
{
    HalDacInstance_t *instance;
    HalDacStatus_t status;

    if (channel >= HAL_DAC_CHANNEL_COUNT)
    {
        return HAL_DAC_STATUS_INVALID_ARGUMENT;
    }
    instance = &g_hal_dac[channel];
    if (!instance->registered)
    {
        return HAL_DAC_STATUS_NOT_REGISTERED;
    }

    status = instance->ops->initialize(instance->driver_context);
    if (status == HAL_DAC_STATUS_OK)
    {
        instance->last_code = 0U;
        instance->initialized = true;
    }
    return status;
}

HalDacStatus_t HalDac_WriteCode(uint8_t channel, uint16_t code)
{
    HalDacInstance_t *instance;
    HalDacStatus_t status;

    if (channel >= HAL_DAC_CHANNEL_COUNT)
    {
        return HAL_DAC_STATUS_INVALID_ARGUMENT;
    }
    instance = &g_hal_dac[channel];
    if (!instance->registered)
    {
        return HAL_DAC_STATUS_NOT_REGISTERED;
    }
    if (!instance->initialized)
    {
        return HAL_DAC_STATUS_NOT_INITIALIZED;
    }

    status = instance->ops->write_code(instance->driver_context, code);
    if (status == HAL_DAC_STATUS_OK)
    {
        instance->last_code = code;
    }
    return status;
}

HalDacStatus_t HalDac_GetLastCode(uint8_t channel, uint16_t *code)
{
    const HalDacInstance_t *instance;

    if ((channel >= HAL_DAC_CHANNEL_COUNT) || (code == NULL))
    {
        return HAL_DAC_STATUS_INVALID_ARGUMENT;
    }
    instance = &g_hal_dac[channel];
    if (!instance->registered)
    {
        return HAL_DAC_STATUS_NOT_REGISTERED;
    }
    if (!instance->initialized)
    {
        return HAL_DAC_STATUS_NOT_INITIALIZED;
    }
    *code = instance->last_code;
    return HAL_DAC_STATUS_OK;
}

bool HalDac_IsInitialized(uint8_t channel)
{
    return (channel < HAL_DAC_CHANNEL_COUNT) &&
           g_hal_dac[channel].registered &&
           g_hal_dac[channel].initialized;
}
