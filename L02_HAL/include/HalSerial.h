#ifndef HAL_SERIAL_H
#define HAL_SERIAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HAL_SERIAL_PORT_COUNT        (2U)
#define HAL_SERIAL_BAUD_RATE_MIN     (4800UL)
#define HAL_SERIAL_BAUD_RATE_MAX     (115200UL)

typedef enum
{
    HAL_SERIAL_PORT_0 = 0,
    HAL_SERIAL_PORT_1 = 1
} HalSerialPort_t;

typedef enum
{
    HAL_SERIAL_STATUS_OK = 0,
    HAL_SERIAL_STATUS_INVALID_ARGUMENT,
    HAL_SERIAL_STATUS_NOT_REGISTERED,
    HAL_SERIAL_STATUS_NOT_CONFIGURED,
    HAL_SERIAL_STATUS_BUSY,
    HAL_SERIAL_STATUS_IO_ERROR,
    HAL_SERIAL_STATUS_UNSUPPORTED
} HalSerialStatus_t;

typedef enum
{
    HAL_SERIAL_DATA_BITS_7 = 7,
    HAL_SERIAL_DATA_BITS_8 = 8
} HalSerialDataBits_t;

typedef enum
{
    HAL_SERIAL_PARITY_NONE = 0,
    HAL_SERIAL_PARITY_EVEN,
    HAL_SERIAL_PARITY_ODD
} HalSerialParity_t;

typedef enum
{
    HAL_SERIAL_STOP_BITS_1 = 1,
    HAL_SERIAL_STOP_BITS_2 = 2
} HalSerialStopBits_t;

typedef enum
{
    HAL_SERIAL_ERROR_NONE    = 0U,
    HAL_SERIAL_ERROR_OVERRUN = (1UL << 0),
    HAL_SERIAL_ERROR_PARITY  = (1UL << 1),
    HAL_SERIAL_ERROR_FRAMING = (1UL << 2),
    HAL_SERIAL_ERROR_NOISE   = (1UL << 3),
    HAL_SERIAL_ERROR_DRIVER  = (1UL << 4)
} HalSerialError_t;

typedef struct
{
    uint32_t baud_rate;
    HalSerialDataBits_t data_bits;
    HalSerialParity_t parity;
    HalSerialStopBits_t stop_bits;
} HalSerialConfig_t;

typedef void (*HalSerialRxCallback_t)(HalSerialPort_t port,
                                      const uint8_t *data,
                                      size_t length,
                                      void *callback_context);
typedef void (*HalSerialTxCallback_t)(HalSerialPort_t port,
                                      void *callback_context);
typedef void (*HalSerialErrorCallback_t)(HalSerialPort_t port,
                                         uint32_t error_mask,
                                         void *callback_context);

typedef struct
{
    HalSerialStatus_t (*initialize)(void *driver_context,
                                    const HalSerialConfig_t *config);
    HalSerialStatus_t (*configure)(void *driver_context,
                                   const HalSerialConfig_t *config);
    HalSerialStatus_t (*write_async)(void *driver_context,
                                     const uint8_t *data,
                                     size_t length);
    HalSerialStatus_t (*abort_write)(void *driver_context);
    bool (*is_write_busy)(void *driver_context);
} HalSerialDriverOps_t;

typedef struct
{
    HalSerialRxCallback_t on_receive;
    HalSerialTxCallback_t on_transmit_complete;
    HalSerialErrorCallback_t on_error;
    void *context;
} HalSerialCallbacks_t;

bool HalSerial_IsConfigValid(const HalSerialConfig_t *config);
HalSerialStatus_t HalSerial_RegisterDriver(HalSerialPort_t port,
                                           const HalSerialDriverOps_t *ops,
                                           void *driver_context);
HalSerialStatus_t HalSerial_Initialize(HalSerialPort_t port,
                                       const HalSerialConfig_t *config,
                                       const HalSerialCallbacks_t *callbacks);
HalSerialStatus_t HalSerial_Configure(HalSerialPort_t port,
                                      const HalSerialConfig_t *config);
HalSerialStatus_t HalSerial_WriteAsync(HalSerialPort_t port,
                                       const uint8_t *data,
                                       size_t length);
HalSerialStatus_t HalSerial_AbortWrite(HalSerialPort_t port);
bool HalSerial_IsWriteBusy(HalSerialPort_t port);

/* Called by the product-owned USART ISR/DMA completion glue. */
void HalSerial_NotifyReceiveFromIsr(HalSerialPort_t port,
                                    const uint8_t *data,
                                    size_t length);
void HalSerial_NotifyTransmitCompleteFromIsr(HalSerialPort_t port);
void HalSerial_NotifyErrorFromIsr(HalSerialPort_t port,
                                  uint32_t error_mask);

#ifdef __cplusplus
}
#endif

#endif
