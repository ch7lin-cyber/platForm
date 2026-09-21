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

typedef struct
{
    uint32_t raw_code;
    uint8_t channel;
} HalAdcSample_t;

typedef struct
{
    HalAdcStatus_t (*initialize)(void *driver_context);
    HalAdcStatus_t (*try_read)(void *driver_context,
                               HalAdcSample_t *sample);
} HalAdcDriverOps_t;

HalAdcStatus_t HalAdc_RegisterDriver(uint8_t device,
                                     const HalAdcDriverOps_t *ops,
                                     void *driver_context);
HalAdcStatus_t HalAdc_Initialize(uint8_t device);
HalAdcStatus_t HalAdc_TryRead(uint8_t device, HalAdcSample_t *sample);
bool HalAdc_IsInitialized(uint8_t device);

#ifdef __cplusplus
}
#endif

#endif
