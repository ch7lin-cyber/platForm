#ifndef HAL_ADC_H
#define HAL_ADC_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ADC_DEVICE_COUNT       (4U)
#define HAL_ADC_CHANNELS_PER_DEVICE (16U)
#define HAL_ADC_RAW_CODE_MAX       (0x00FFFFFFUL)
#define HAL_ADC_SETUP_COUNT        (8U)

typedef enum
{
    HAL_ADC_STATUS_OK = 0,
    HAL_ADC_STATUS_INVALID_ARGUMENT,
    HAL_ADC_STATUS_NOT_REGISTERED,
    HAL_ADC_STATUS_NOT_INITIALIZED,
    HAL_ADC_STATUS_NOT_READY,
    HAL_ADC_STATUS_IO_ERROR,
    HAL_ADC_STATUS_DEVICE_ERROR
} HalAdcStatus_t;

typedef enum
{
    HAL_ADC_REFERENCE_EXTERNAL_1 = 0,
    HAL_ADC_REFERENCE_EXTERNAL_2,
    HAL_ADC_REFERENCE_INTERNAL,
    HAL_ADC_REFERENCE_SUPPLY
} HalAdcReference_t;

typedef enum
{
    HAL_ADC_FILTER_SINC4 = 0,
    HAL_ADC_FILTER_SINC3,
    HAL_ADC_FILTER_FAST_SINC4,
    HAL_ADC_FILTER_POST
} HalAdcFilterType_t;

typedef enum
{
    HAL_ADC_GAIN_1 = 1,
    HAL_ADC_GAIN_2 = 2,
    HAL_ADC_GAIN_4 = 4,
    HAL_ADC_GAIN_8 = 8,
    HAL_ADC_GAIN_16 = 16,
    HAL_ADC_GAIN_32 = 32,
    HAL_ADC_GAIN_64 = 64,
    HAL_ADC_GAIN_128 = 128
} HalAdcGain_t;

typedef struct
{
    HalAdcReference_t reference;
    HalAdcFilterType_t filter;
    HalAdcGain_t gain;
    uint16_t filter_word;
    bool bipolar;
    bool input_buffer_enabled;
    bool reference_buffer_enabled;
} HalAdcSetupConfig_t;

typedef struct
{
    uint8_t channel;
    uint8_t setup;
    uint8_t positive_input;
    uint8_t negative_input;
    bool enabled;
} HalAdcChannelConfig_t;

typedef struct
{
    const HalAdcSetupConfig_t *setups;
    uint8_t setup_count;
    const HalAdcChannelConfig_t *channels;
    uint8_t channel_count;
} HalAdcDeviceConfig_t;

typedef struct
{
    uint32_t raw_code;
    int32_t microvolts;
    uint8_t channel;
} HalAdcSample_t;

typedef struct
{
    HalAdcStatus_t (*initialize)(void *driver_context);
    HalAdcStatus_t (*configure)(void *driver_context,
                                const HalAdcDeviceConfig_t *config);
    HalAdcStatus_t (*try_read)(void *driver_context,
                               HalAdcSample_t *sample);
} HalAdcDriverOps_t;

HalAdcStatus_t HalAdc_RegisterDriver(uint8_t device,
                                     const HalAdcDriverOps_t *ops,
                                     void *driver_context);
HalAdcStatus_t HalAdc_Initialize(uint8_t device);
HalAdcStatus_t HalAdc_Configure(uint8_t device,
                                const HalAdcDeviceConfig_t *config);
HalAdcStatus_t HalAdc_TryRead(uint8_t device, HalAdcSample_t *sample);
bool HalAdc_IsInitialized(uint8_t device);

#ifdef __cplusplus
}
#endif

#endif
