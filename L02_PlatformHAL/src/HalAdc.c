#include "HalAdc.h"

#include <stddef.h>
#include <string.h>

typedef struct
{
    const HalAdcDriverOps_t *ops;
    void *driver_context;
    bool registered;
    bool initialized;
} HalAdcInstance_t;

static HalAdcInstance_t g_hal_adc[HAL_ADC_DEVICE_COUNT];

HalAdcStatus_t HalAdc_RegisterDriver(uint8_t device,
                                     const HalAdcDriverOps_t *ops,
                                     void *driver_context)
{
    HalAdcInstance_t *instance;

    if ((device >= HAL_ADC_DEVICE_COUNT) || (ops == NULL) ||
        (ops->initialize == NULL) || (ops->try_read == NULL))
    {
        return HAL_ADC_STATUS_INVALID_ARGUMENT;
    }

    instance = &g_hal_adc[device];
    (void)memset(instance, 0, sizeof(*instance));
    instance->ops = ops;
    instance->driver_context = driver_context;
    instance->registered = true;
    return HAL_ADC_STATUS_OK;
}

HalAdcStatus_t HalAdc_Initialize(uint8_t device)
{
    HalAdcInstance_t *instance;
    HalAdcStatus_t status;

    if (device >= HAL_ADC_DEVICE_COUNT)
    {
        return HAL_ADC_STATUS_INVALID_ARGUMENT;
    }
    instance = &g_hal_adc[device];
    if (!instance->registered)
    {
        return HAL_ADC_STATUS_NOT_REGISTERED;
    }

    status = instance->ops->initialize(instance->driver_context);
    instance->initialized = (status == HAL_ADC_STATUS_OK);
    return status;
}

HalAdcStatus_t HalAdc_TryRead(uint8_t device, HalAdcSample_t *sample)
{
    HalAdcInstance_t *instance;

    if ((device >= HAL_ADC_DEVICE_COUNT) || (sample == NULL))
    {
        return HAL_ADC_STATUS_INVALID_ARGUMENT;
    }
    instance = &g_hal_adc[device];
    if (!instance->registered)
    {
        return HAL_ADC_STATUS_NOT_REGISTERED;
    }
    if (!instance->initialized)
    {
        return HAL_ADC_STATUS_NOT_INITIALIZED;
    }
    return instance->ops->try_read(instance->driver_context, sample);
}

bool HalAdc_IsInitialized(uint8_t device)
{
    return (device < HAL_ADC_DEVICE_COUNT) &&
           g_hal_adc[device].registered &&
           g_hal_adc[device].initialized;
}
