#ifndef SERIAL_SERVICE_H
#define SERIAL_SERVICE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "HalSerial.h"
#include "SerialConfiguration.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SERIAL_SERVICE_RX_CAPACITY   (1024U)
#define SERIAL_SERVICE_TX_CAPACITY   (520U)

typedef enum
{
    SERIAL_SERVICE_STATUS_OK = 0,
    SERIAL_SERVICE_STATUS_INVALID_ARGUMENT,
    SERIAL_SERVICE_STATUS_NOT_INITIALIZED,
    SERIAL_SERVICE_STATUS_BUSY,
    SERIAL_SERVICE_STATUS_BUFFER_TOO_SMALL,
    SERIAL_SERVICE_STATUS_HAL_ERROR
} SerialServiceStatus_t;

typedef enum
{
    SERIAL_SERVICE_EVENT_RX_AVAILABLE = (1UL << 0),
    SERIAL_SERVICE_EVENT_RX_IDLE      = (1UL << 1),
    SERIAL_SERVICE_EVENT_TX_COMPLETE  = (1UL << 2),
    SERIAL_SERVICE_EVENT_RX_OVERFLOW  = (1UL << 3),
    SERIAL_SERVICE_EVENT_LINE_ERROR   = (1UL << 4),
    SERIAL_SERVICE_EVENT_RECONFIGURED = (1UL << 5)
} SerialServiceEvent_t;

typedef struct
{
    uint32_t received_bytes;
    uint32_t transmitted_bytes;
    uint32_t dropped_rx_bytes;
    uint32_t line_error_mask;
    uint32_t line_error_count;
    size_t rx_pending_bytes;
    bool transmit_busy;
} SerialServiceStatistics_t;

typedef void (*SerialServiceEventCallback_t)(HalSerialPort_t port,
                                             uint32_t event_mask,
                                             void *callback_context);

SerialServiceStatus_t SerialService_InitializePort(
    HalSerialPort_t port,
    const SerialConfiguration_t *configuration,
    SerialServiceEventCallback_t event_callback,
    void *callback_context);
SerialServiceStatus_t SerialService_ConfigurePort(
    HalSerialPort_t port,
    const SerialConfiguration_t *configuration);
SerialServiceStatus_t SerialService_Write(HalSerialPort_t port,
                                          const uint8_t *data,
                                          size_t length);
size_t SerialService_Read(HalSerialPort_t port,
                          uint8_t *data,
                          size_t maximum_length);
size_t SerialService_GetAvailable(HalSerialPort_t port);
void SerialService_FlushReceive(HalSerialPort_t port);
void SerialService_Tick1ms(void);
void SerialService_Process(void);
bool SerialService_GetConfiguration(HalSerialPort_t port,
                                    SerialConfiguration_t *configuration);
bool SerialService_GetStatistics(HalSerialPort_t port,
                                 SerialServiceStatistics_t *statistics);
void SerialService_ClearStatistics(HalSerialPort_t port);

#ifdef __cplusplus
}
#endif

#endif
