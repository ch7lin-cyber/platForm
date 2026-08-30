#include "SerialService.h"

#include <limits.h>
#include <string.h>

#include "SerialRingBuffer.h"

#define SERIAL_SERVICE_RX_STORAGE_SIZE (SERIAL_SERVICE_RX_CAPACITY + 1U)

typedef struct
{
    SerialRingBuffer_t rx_buffer;
    uint8_t rx_storage[SERIAL_SERVICE_RX_STORAGE_SIZE];
    uint8_t tx_storage[SERIAL_SERVICE_TX_CAPACITY];
    SerialConfiguration_t configuration;
    SerialServiceEventCallback_t event_callback;
    void *callback_context;
    volatile uint32_t pending_events;
    volatile uint32_t idle_time_us;
    volatile uint32_t received_bytes;
    volatile uint32_t transmitted_bytes;
    volatile uint32_t dropped_rx_bytes;
    volatile uint32_t line_error_mask;
    volatile uint32_t line_error_count;
    size_t active_tx_length;
    bool initialized;
    volatile bool transmit_busy;
    volatile bool idle_event_reported;
} SerialServiceInstance_t;

static SerialServiceInstance_t g_serial_service[HAL_SERIAL_PORT_COUNT];

static bool SerialService_IsPortValid(HalSerialPort_t port)
{
    return ((uint32_t)port < HAL_SERIAL_PORT_COUNT);
}

static SerialServiceStatus_t SerialService_ConvertHalStatus(
    HalSerialStatus_t status)
{
    if (status == HAL_SERIAL_STATUS_OK)
    {
        return SERIAL_SERVICE_STATUS_OK;
    }
    if (status == HAL_SERIAL_STATUS_BUSY)
    {
        return SERIAL_SERVICE_STATUS_BUSY;
    }
    if (status == HAL_SERIAL_STATUS_INVALID_ARGUMENT)
    {
        return SERIAL_SERVICE_STATUS_INVALID_ARGUMENT;
    }
    return SERIAL_SERVICE_STATUS_HAL_ERROR;
}

static void SerialService_OnReceive(HalSerialPort_t port,
                                    const uint8_t *data,
                                    size_t length,
                                    void *callback_context)
{
    SerialServiceInstance_t *instance = (SerialServiceInstance_t *)callback_context;
    size_t written;

    (void)port;
    written = SerialRingBuffer_PushBlockFromIsr(&instance->rx_buffer,
                                                data, length);
    instance->received_bytes += (uint32_t)written;
    instance->idle_time_us = 0U;
    instance->idle_event_reported = false;
    if (written > 0U)
    {
        instance->pending_events |= SERIAL_SERVICE_EVENT_RX_AVAILABLE;
    }
    if (written < length)
    {
        instance->dropped_rx_bytes += (uint32_t)(length - written);
        instance->pending_events |= SERIAL_SERVICE_EVENT_RX_OVERFLOW;
    }
}

static void SerialService_OnTransmitComplete(HalSerialPort_t port,
                                             void *callback_context)
{
    SerialServiceInstance_t *instance = (SerialServiceInstance_t *)callback_context;

    (void)port;
    instance->transmitted_bytes += (uint32_t)instance->active_tx_length;
    instance->active_tx_length = 0U;
    instance->transmit_busy = false;
    instance->pending_events |= SERIAL_SERVICE_EVENT_TX_COMPLETE;
}

static void SerialService_OnError(HalSerialPort_t port,
                                  uint32_t error_mask,
                                  void *callback_context)
{
    SerialServiceInstance_t *instance = (SerialServiceInstance_t *)callback_context;

    (void)port;
    instance->line_error_mask |= error_mask;
    instance->line_error_count++;
    instance->pending_events |= SERIAL_SERVICE_EVENT_LINE_ERROR;
}

SerialServiceStatus_t SerialService_InitializePort(
    HalSerialPort_t port,
    const SerialConfiguration_t *configuration,
    SerialServiceEventCallback_t event_callback,
    void *callback_context)
{
    SerialServiceInstance_t *instance;
    HalSerialCallbacks_t callbacks;
    HalSerialStatus_t hal_status;

    if ((!SerialService_IsPortValid(port)) ||
        (!SerialConfiguration_IsValid(configuration)))
    {
        return SERIAL_SERVICE_STATUS_INVALID_ARGUMENT;
    }

    instance = &g_serial_service[(uint32_t)port];
    (void)memset(instance, 0, sizeof(*instance));
    if (!SerialRingBuffer_Initialize(&instance->rx_buffer,
                                     instance->rx_storage,
                                     sizeof(instance->rx_storage)))
    {
        return SERIAL_SERVICE_STATUS_HAL_ERROR;
    }

    callbacks.on_receive = SerialService_OnReceive;
    callbacks.on_transmit_complete = SerialService_OnTransmitComplete;
    callbacks.on_error = SerialService_OnError;
    callbacks.context = instance;

    hal_status = HalSerial_Initialize(port, &configuration->line, &callbacks);
    if (hal_status != HAL_SERIAL_STATUS_OK)
    {
        return SerialService_ConvertHalStatus(hal_status);
    }

    instance->configuration = *configuration;
    instance->event_callback = event_callback;
    instance->callback_context = callback_context;
    instance->initialized = true;
    return SERIAL_SERVICE_STATUS_OK;
}

SerialServiceStatus_t SerialService_ConfigurePort(
    HalSerialPort_t port,
    const SerialConfiguration_t *configuration)
{
    SerialServiceInstance_t *instance;
    HalSerialStatus_t hal_status;

    if ((!SerialService_IsPortValid(port)) ||
        (!SerialConfiguration_IsValid(configuration)))
    {
        return SERIAL_SERVICE_STATUS_INVALID_ARGUMENT;
    }
    instance = &g_serial_service[(uint32_t)port];
    if (!instance->initialized)
    {
        return SERIAL_SERVICE_STATUS_NOT_INITIALIZED;
    }
    if (instance->transmit_busy || HalSerial_IsWriteBusy(port))
    {
        return SERIAL_SERVICE_STATUS_BUSY;
    }

    hal_status = HalSerial_Configure(port, &configuration->line);
    if (hal_status == HAL_SERIAL_STATUS_OK)
    {
        instance->configuration = *configuration;
        SerialRingBuffer_Clear(&instance->rx_buffer);
        instance->idle_time_us = 0U;
        instance->idle_event_reported = false;
        instance->pending_events |= SERIAL_SERVICE_EVENT_RECONFIGURED;
    }
    return SerialService_ConvertHalStatus(hal_status);
}

SerialServiceStatus_t SerialService_Write(HalSerialPort_t port,
                                          const uint8_t *data,
                                          size_t length)
{
    SerialServiceInstance_t *instance;
    HalSerialStatus_t hal_status;

    if ((!SerialService_IsPortValid(port)) || (data == NULL) || (length == 0U))
    {
        return SERIAL_SERVICE_STATUS_INVALID_ARGUMENT;
    }
    if (length > SERIAL_SERVICE_TX_CAPACITY)
    {
        return SERIAL_SERVICE_STATUS_BUFFER_TOO_SMALL;
    }
    instance = &g_serial_service[(uint32_t)port];
    if (!instance->initialized)
    {
        return SERIAL_SERVICE_STATUS_NOT_INITIALIZED;
    }
    if (instance->transmit_busy || HalSerial_IsWriteBusy(port))
    {
        return SERIAL_SERVICE_STATUS_BUSY;
    }

    (void)memcpy(instance->tx_storage, data, length);
    instance->active_tx_length = length;
    instance->transmit_busy = true;
    hal_status = HalSerial_WriteAsync(port, instance->tx_storage, length);
    if (hal_status != HAL_SERIAL_STATUS_OK)
    {
        instance->active_tx_length = 0U;
        instance->transmit_busy = false;
    }
    return SerialService_ConvertHalStatus(hal_status);
}

size_t SerialService_Read(HalSerialPort_t port,
                          uint8_t *data,
                          size_t maximum_length)
{
    SerialServiceInstance_t *instance;

    if ((!SerialService_IsPortValid(port)) || (data == NULL) ||
        (maximum_length == 0U))
    {
        return 0U;
    }
    instance = &g_serial_service[(uint32_t)port];
    if (!instance->initialized)
    {
        return 0U;
    }
    return SerialRingBuffer_Read(&instance->rx_buffer, data, maximum_length);
}

size_t SerialService_GetAvailable(HalSerialPort_t port)
{
    if ((!SerialService_IsPortValid(port)) ||
        (!g_serial_service[(uint32_t)port].initialized))
    {
        return 0U;
    }
    return SerialRingBuffer_Count(&g_serial_service[(uint32_t)port].rx_buffer);
}

void SerialService_FlushReceive(HalSerialPort_t port)
{
    if (SerialService_IsPortValid(port) &&
        g_serial_service[(uint32_t)port].initialized)
    {
        SerialRingBuffer_Clear(&g_serial_service[(uint32_t)port].rx_buffer);
    }
}

void SerialService_Tick1ms(void)
{
    uint32_t index;

    for (index = 0U; index < HAL_SERIAL_PORT_COUNT; index++)
    {
        SerialServiceInstance_t *instance = &g_serial_service[index];
        if (instance->initialized && (instance->idle_time_us <= (UINT32_MAX - 1000U)))
        {
            instance->idle_time_us += 1000U;
        }
    }
}

void SerialService_Process(void)
{
    uint32_t index;

    for (index = 0U; index < HAL_SERIAL_PORT_COUNT; index++)
    {
        SerialServiceInstance_t *instance = &g_serial_service[index];
        uint32_t events;

        if (!instance->initialized)
        {
            continue;
        }

        if ((!instance->idle_event_reported) &&
            (SerialRingBuffer_Count(&instance->rx_buffer) > 0U) &&
            (instance->configuration.protocol == SERIAL_PROTOCOL_MODBUS_RTU) &&
            (instance->idle_time_us >= SerialConfiguration_GetRtuFrameGapUs(
                                           &instance->configuration)))
        {
            instance->idle_event_reported = true;
            instance->pending_events |= SERIAL_SERVICE_EVENT_RX_IDLE;
        }

        events = instance->pending_events;
        instance->pending_events = 0U;
        if ((events != 0U) && (instance->event_callback != NULL))
        {
            instance->event_callback((HalSerialPort_t)index, events,
                                     instance->callback_context);
        }
    }
}

bool SerialService_GetConfiguration(HalSerialPort_t port,
                                    SerialConfiguration_t *configuration)
{
    if ((!SerialService_IsPortValid(port)) || (configuration == NULL) ||
        (!g_serial_service[(uint32_t)port].initialized))
    {
        return false;
    }
    *configuration = g_serial_service[(uint32_t)port].configuration;
    return true;
}

bool SerialService_GetStatistics(HalSerialPort_t port,
                                 SerialServiceStatistics_t *statistics)
{
    const SerialServiceInstance_t *instance;

    if ((!SerialService_IsPortValid(port)) || (statistics == NULL))
    {
        return false;
    }
    instance = &g_serial_service[(uint32_t)port];
    if (!instance->initialized)
    {
        return false;
    }
    statistics->received_bytes = instance->received_bytes;
    statistics->transmitted_bytes = instance->transmitted_bytes;
    statistics->dropped_rx_bytes = instance->dropped_rx_bytes;
    statistics->line_error_mask = instance->line_error_mask;
    statistics->line_error_count = instance->line_error_count;
    statistics->rx_pending_bytes = SerialRingBuffer_Count(&instance->rx_buffer);
    statistics->transmit_busy = instance->transmit_busy;
    return true;
}

void SerialService_ClearStatistics(HalSerialPort_t port)
{
    SerialServiceInstance_t *instance;

    if ((!SerialService_IsPortValid(port)) ||
        (!g_serial_service[(uint32_t)port].initialized))
    {
        return;
    }
    instance = &g_serial_service[(uint32_t)port];
    instance->received_bytes = 0U;
    instance->transmitted_bytes = 0U;
    instance->dropped_rx_bytes = 0U;
    instance->line_error_mask = 0U;
    instance->line_error_count = 0U;
}
