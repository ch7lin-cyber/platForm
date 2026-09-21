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

static bool IsSupportedGain(HalAdcGain_t gain)
{
    return (gain == HAL_ADC_GAIN_1) || (gain == HAL_ADC_GAIN_2) ||
           (gain == HAL_ADC_GAIN_4) || (gain == HAL_ADC_GAIN_8) ||
           (gain == HAL_ADC_GAIN_16) || (gain == HAL_ADC_GAIN_32) ||
           (gain == HAL_ADC_GAIN_64) || (gain == HAL_ADC_GAIN_128);
}

HalAdcStatus_t HalAdc_RegisterDriver(uint8_t device,
                                     const HalAdcDriverOps_t *ops,
                                     void *driver_context)
{
    HalAdcInstance_t *instance;

    if ((device >= HAL_ADC_DEVICE_COUNT) || (ops == NULL) ||
        (ops->initialize == NULL) || (ops->configure == NULL) ||
        (ops->try_read == NULL))
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

HalAdcStatus_t HalAdc_Configure(uint8_t device,
                                const HalAdcDeviceConfig_t *config)
{
    HalAdcInstance_t *instance;
    uint8_t index;

    if ((device >= HAL_ADC_DEVICE_COUNT) || (config == NULL) ||
        (config->setups == NULL) || (config->setup_count == 0U) ||
        (config->setup_count > HAL_ADC_SETUP_COUNT) ||
        (config->channels == NULL) || (config->channel_count == 0U) ||
        (config->channel_count > HAL_ADC_CHANNELS_PER_DEVICE))
    {
        return HAL_ADC_STATUS_INVALID_ARGUMENT;
    }
    for (index = 0U; index < config->setup_count; index++)
    {
        if ((config->setups[index].reference > HAL_ADC_REFERENCE_SUPPLY) ||
            (config->setups[index].filter > HAL_ADC_FILTER_POST) ||
            !IsSupportedGain(config->setups[index].gain) ||
            (config->setups[index].filter_word > 0x07FFU))
        {
            return HAL_ADC_STATUS_INVALID_ARGUMENT;
        }
    }
    for (index = 0U; index < config->channel_count; index++)
    {
        if ((config->channels[index].channel >=
             HAL_ADC_CHANNELS_PER_DEVICE) ||
            (config->channels[index].setup >= config->setup_count) ||
            (config->channels[index].positive_input > 31U) ||
            (config->channels[index].negative_input > 31U))
        {
            return HAL_ADC_STATUS_INVALID_ARGUMENT;
        }
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
    return instance->ops->configure(instance->driver_context, config);
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
