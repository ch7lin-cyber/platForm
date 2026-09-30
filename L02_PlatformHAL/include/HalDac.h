#ifndef HAL_DAC_H
#define HAL_DAC_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HAL_DAC_CHANNEL_COUNT (16U)

typedef enum
{
    HAL_DAC_STATUS_OK = 0,
    HAL_DAC_STATUS_INVALID_ARGUMENT,
    HAL_DAC_STATUS_NOT_REGISTERED,
    HAL_DAC_STATUS_NOT_INITIALIZED,
    HAL_DAC_STATUS_IO_ERROR
} HalDacStatus_t;

typedef struct
{
    HalDacStatus_t (*initialize)(void *driver_context);
    HalDacStatus_t (*write_code)(void *driver_context, uint16_t code);
} HalDacDriverOps_t;

HalDacStatus_t HalDac_RegisterDriver(uint8_t channel,
                                    const HalDacDriverOps_t *ops,
                                    void *driver_context);
HalDacStatus_t HalDac_Initialize(uint8_t channel);
HalDacStatus_t HalDac_WriteCode(uint8_t channel, uint16_t code);
HalDacStatus_t HalDac_GetLastCode(uint8_t channel, uint16_t *code);
bool HalDac_IsInitialized(uint8_t channel);

#ifdef __cplusplus
}
#endif

#endif
