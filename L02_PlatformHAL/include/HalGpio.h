#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HAL_GPIO_INPUT_CHANNEL_COUNT  (16U)
#define HAL_GPIO_OUTPUT_CHANNEL_COUNT (16U)

typedef enum
{
    HAL_GPIO_STATUS_OK = 0,
    HAL_GPIO_STATUS_INVALID_ARGUMENT,
    HAL_GPIO_STATUS_NOT_REGISTERED,
    HAL_GPIO_STATUS_NOT_INITIALIZED,
    HAL_GPIO_STATUS_IO_ERROR
} HalGpioStatus_t;

typedef struct
{
    HalGpioStatus_t (*initialize)(void *driver_context);
    HalGpioStatus_t (*read)(void *driver_context, bool *active);
} HalGpioInputDriverOps_t;

typedef struct
{
    HalGpioStatus_t (*initialize)(void *driver_context);
    HalGpioStatus_t (*write)(void *driver_context, bool active);
} HalGpioOutputDriverOps_t;

HalGpioStatus_t HalGpio_RegisterInputDriver(
    uint8_t channel,
    const HalGpioInputDriverOps_t *ops,
    void *driver_context);
HalGpioStatus_t HalGpio_RegisterOutputDriver(
    uint8_t channel,
    const HalGpioOutputDriverOps_t *ops,
    void *driver_context);
HalGpioStatus_t HalGpio_InitializeInput(uint8_t channel);
HalGpioStatus_t HalGpio_InitializeOutput(uint8_t channel);
HalGpioStatus_t HalGpio_ReadInput(uint8_t channel, bool *active);
HalGpioStatus_t HalGpio_WriteOutput(uint8_t channel, bool active);
bool HalGpio_IsInputInitialized(uint8_t channel);
bool HalGpio_IsOutputInitialized(uint8_t channel);

#ifdef __cplusplus
}
#endif

#endif
