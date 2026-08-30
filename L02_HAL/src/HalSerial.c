#include "HalSerial.h"

#include <string.h>

typedef struct
{
    const HalSerialDriverOps_t *ops;
    void *driver_context;
    HalSerialCallbacks_t callbacks;
    HalSerialConfig_t config;
    bool registered;
    bool configured;
} HalSerialInstance_t;

static HalSerialInstance_t g_hal_serial[HAL_SERIAL_PORT_COUNT];

static bool HalSerial_IsPortValid(HalSerialPort_t port)
{
    return ((uint32_t)port < HAL_SERIAL_PORT_COUNT);
}

bool HalSerial_IsConfigValid(const HalSerialConfig_t *config)
{
    if (config == NULL)
    {
        return false;
    }

    if ((config->baud_rate < HAL_SERIAL_BAUD_RATE_MIN) ||
        (config->baud_rate > HAL_SERIAL_BAUD_RATE_MAX))
    {
        return false;
    }

    if ((config->data_bits != HAL_SERIAL_DATA_BITS_7) &&
        (config->data_bits != HAL_SERIAL_DATA_BITS_8))
    {
        return false;
    }

    if ((config->parity != HAL_SERIAL_PARITY_NONE) &&
        (config->parity != HAL_SERIAL_PARITY_EVEN) &&
        (config->parity != HAL_SERIAL_PARITY_ODD))
    {
        return false;
    }

    return ((config->stop_bits == HAL_SERIAL_STOP_BITS_1) ||
            (config->stop_bits == HAL_SERIAL_STOP_BITS_2));
}

HalSerialStatus_t HalSerial_RegisterDriver(HalSerialPort_t port,
                                           const HalSerialDriverOps_t *ops,
                                           void *driver_context)
{
    HalSerialInstance_t *instance;

    if ((!HalSerial_IsPortValid(port)) || (ops == NULL) ||
        (ops->initialize == NULL) || (ops->configure == NULL) ||
        (ops->write_async == NULL) || (ops->is_write_busy == NULL))
    {
        return HAL_SERIAL_STATUS_INVALID_ARGUMENT;
    }

    instance = &g_hal_serial[(uint32_t)port];
    (void)memset(instance, 0, sizeof(*instance));
    instance->ops = ops;
    instance->driver_context = driver_context;
    instance->registered = true;
    return HAL_SERIAL_STATUS_OK;
}

HalSerialStatus_t HalSerial_Initialize(HalSerialPort_t port,
                                       const HalSerialConfig_t *config,
                                       const HalSerialCallbacks_t *callbacks)
{
    HalSerialInstance_t *instance;
    HalSerialStatus_t status;

    if ((!HalSerial_IsPortValid(port)) ||
        (!HalSerial_IsConfigValid(config)) || (callbacks == NULL))
    {
        return HAL_SERIAL_STATUS_INVALID_ARGUMENT;
    }

    instance = &g_hal_serial[(uint32_t)port];
    if (!instance->registered)
    {
        return HAL_SERIAL_STATUS_NOT_REGISTERED;
    }

    status = instance->ops->initialize(instance->driver_context, config);
    if (status == HAL_SERIAL_STATUS_OK)
    {
        instance->config = *config;
        instance->callbacks = *callbacks;
        instance->configured = true;
    }
    return status;
}

HalSerialStatus_t HalSerial_Configure(HalSerialPort_t port,
                                      const HalSerialConfig_t *config)
{
    HalSerialInstance_t *instance;
    HalSerialStatus_t status;

    if ((!HalSerial_IsPortValid(port)) || (!HalSerial_IsConfigValid(config)))
    {
        return HAL_SERIAL_STATUS_INVALID_ARGUMENT;
    }

    instance = &g_hal_serial[(uint32_t)port];
    if (!instance->registered)
    {
        return HAL_SERIAL_STATUS_NOT_REGISTERED;
    }
    if (instance->configured && instance->ops->is_write_busy(instance->driver_context))
    {
        return HAL_SERIAL_STATUS_BUSY;
    }

    status = instance->ops->configure(instance->driver_context, config);
    if (status == HAL_SERIAL_STATUS_OK)
    {
        instance->config = *config;
        instance->configured = true;
    }
    return status;
}

HalSerialStatus_t HalSerial_WriteAsync(HalSerialPort_t port,
                                       const uint8_t *data,
                                       size_t length)
{
    HalSerialInstance_t *instance;

    if ((!HalSerial_IsPortValid(port)) || (data == NULL) || (length == 0U))
    {
        return HAL_SERIAL_STATUS_INVALID_ARGUMENT;
    }

    instance = &g_hal_serial[(uint32_t)port];
    if (!instance->registered)
    {
        return HAL_SERIAL_STATUS_NOT_REGISTERED;
    }
    if (!instance->configured)
    {
        return HAL_SERIAL_STATUS_NOT_CONFIGURED;
    }
    if (instance->ops->is_write_busy(instance->driver_context))
    {
        return HAL_SERIAL_STATUS_BUSY;
    }
    return instance->ops->write_async(instance->driver_context, data, length);
}

HalSerialStatus_t HalSerial_AbortWrite(HalSerialPort_t port)
{
    HalSerialInstance_t *instance;

    if (!HalSerial_IsPortValid(port))
    {
        return HAL_SERIAL_STATUS_INVALID_ARGUMENT;
    }
    instance = &g_hal_serial[(uint32_t)port];
    if (!instance->registered)
    {
        return HAL_SERIAL_STATUS_NOT_REGISTERED;
    }
    if (instance->ops->abort_write == NULL)
    {
        return HAL_SERIAL_STATUS_UNSUPPORTED;
    }
    return instance->ops->abort_write(instance->driver_context);
}

bool HalSerial_IsWriteBusy(HalSerialPort_t port)
{
    const HalSerialInstance_t *instance;

    if (!HalSerial_IsPortValid(port))
    {
        return false;
    }
    instance = &g_hal_serial[(uint32_t)port];
    return (instance->registered && instance->configured &&
            instance->ops->is_write_busy(instance->driver_context));
}

void HalSerial_NotifyReceiveFromIsr(HalSerialPort_t port,
                                    const uint8_t *data,
                                    size_t length)
{
    HalSerialInstance_t *instance;

    if ((!HalSerial_IsPortValid(port)) || (data == NULL) || (length == 0U))
    {
        return;
    }
    instance = &g_hal_serial[(uint32_t)port];
    if (instance->configured && (instance->callbacks.on_receive != NULL))
    {
        instance->callbacks.on_receive(port, data, length,
                                       instance->callbacks.context);
    }
}

void HalSerial_NotifyTransmitCompleteFromIsr(HalSerialPort_t port)
{
    HalSerialInstance_t *instance;

    if (!HalSerial_IsPortValid(port))
    {
        return;
    }
    instance = &g_hal_serial[(uint32_t)port];
    if (instance->configured &&
        (instance->callbacks.on_transmit_complete != NULL))
    {
        instance->callbacks.on_transmit_complete(port,
                                                 instance->callbacks.context);
    }
}

void HalSerial_NotifyErrorFromIsr(HalSerialPort_t port, uint32_t error_mask)
{
    HalSerialInstance_t *instance;

    if (!HalSerial_IsPortValid(port))
    {
        return;
    }
    instance = &g_hal_serial[(uint32_t)port];
    if (instance->configured && (instance->callbacks.on_error != NULL))
    {
        instance->callbacks.on_error(port, error_mask,
                                     instance->callbacks.context);
    }
}
