#include "SerialConfigurationApplyService.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

typedef struct
{
    SerialServiceEventCallback_t event_callback;
    void *callback_context;
    uint32_t apply_response_timeout_ms;
    uint32_t waiting_time_ms;
    bool initialized;
} SerialApplyInstance_t;

static SerialApplyInstance_t g_serial_apply[HAL_SERIAL_PORT_COUNT];

static void CancelWaitingApply(HalSerialPort_t port)
{
    SerialApplyInstance_t *instance = &g_serial_apply[(uint32_t)port];

    if (ModbusRegisterAdapter_IsApplyRequested((uint8_t)port))
    {
        (void)ModbusRegisterAdapter_CancelApply((uint8_t)port);
    }
    instance->waiting_time_ms = 0U;
}

static void ApplyAfterTransmitComplete(HalSerialPort_t port)
{
    SerialApplyInstance_t *instance = &g_serial_apply[(uint32_t)port];
    ModbusSerialPortConfiguration_t configuration;
    SerialServiceStatus_t status;

    if (!ModbusRegisterAdapter_IsApplyRequested((uint8_t)port))
    {
        return;
    }

    if (!ModbusRegisterAdapter_BeginApply((uint8_t)port, &configuration))
    {
        CancelWaitingApply(port);
        return;
    }

    status = SerialService_ConfigurePort(port, &configuration.serial);
    (void)ModbusRegisterAdapter_CompleteApply(
        (uint8_t)port, status == SERIAL_SERVICE_STATUS_OK);
    instance->waiting_time_ms = 0U;
}

static void OnSerialEvent(
    HalSerialPort_t port,
    uint32_t event_mask,
    void *callback_context)
{
    SerialApplyInstance_t *instance =
        (SerialApplyInstance_t *)callback_context;

    if ((event_mask & SERIAL_SERVICE_EVENT_LINE_ERROR) != 0U)
    {
        CancelWaitingApply(port);
    }
    else if ((event_mask & SERIAL_SERVICE_EVENT_TX_COMPLETE) != 0U)
    {
        ApplyAfterTransmitComplete(port);
    }

    if (instance->event_callback != NULL)
    {
        instance->event_callback(port, event_mask,
                                 instance->callback_context);
    }
}

SerialServiceStatus_t SerialConfigurationApplyService_InitializePort(
    HalSerialPort_t port,
    const ModbusSerialPortConfiguration_t *configuration,
    uint32_t apply_response_timeout_ms,
    SerialServiceEventCallback_t event_callback,
    void *callback_context)
{
    SerialApplyInstance_t *instance;
    SerialServiceStatus_t status;

    if (((uint32_t)port >= HAL_SERIAL_PORT_COUNT) ||
        (configuration == NULL) || (apply_response_timeout_ms == 0U))
    {
        return SERIAL_SERVICE_STATUS_INVALID_ARGUMENT;
    }
    if (!ModbusRegisterAdapter_InitializeSerialPort((uint8_t)port,
                                                     configuration))
    {
        return SERIAL_SERVICE_STATUS_INVALID_ARGUMENT;
    }

    instance = &g_serial_apply[(uint32_t)port];
    (void)memset(instance, 0, sizeof(*instance));
    instance->event_callback = event_callback;
    instance->callback_context = callback_context;
    instance->apply_response_timeout_ms = apply_response_timeout_ms;

    status = SerialService_InitializePort(port, &configuration->serial,
                                          OnSerialEvent, instance);
    if (status == SERIAL_SERVICE_STATUS_OK)
    {
        instance->initialized = true;
    }
    return status;
}

SerialServiceStatus_t SerialConfigurationApplyService_WriteResponse(
    HalSerialPort_t port,
    const uint8_t *response,
    size_t response_length)
{
    SerialServiceStatus_t status;

    if (((uint32_t)port >= HAL_SERIAL_PORT_COUNT) ||
        (!g_serial_apply[(uint32_t)port].initialized))
    {
        return SERIAL_SERVICE_STATUS_NOT_INITIALIZED;
    }

    status = SerialService_Write(port, response, response_length);
    if ((status != SERIAL_SERVICE_STATUS_OK) &&
        (status != SERIAL_SERVICE_STATUS_BUSY))
    {
        CancelWaitingApply(port);
    }
    return status;
}

void SerialConfigurationApplyService_Tick1ms(void)
{
    uint32_t index;

    SerialService_Tick1ms();
    for (index = 0U; index < HAL_SERIAL_PORT_COUNT; index++)
    {
        SerialApplyInstance_t *instance = &g_serial_apply[index];

        if (instance->initialized &&
            ModbusRegisterAdapter_IsApplyRequested((uint8_t)index))
        {
            if (instance->waiting_time_ms < UINT32_MAX)
            {
                instance->waiting_time_ms++;
            }
            if (instance->waiting_time_ms >=
                instance->apply_response_timeout_ms)
            {
                CancelWaitingApply((HalSerialPort_t)index);
            }
        }
        else
        {
            instance->waiting_time_ms = 0U;
        }
    }
}

void SerialConfigurationApplyService_Process(void)
{
    SerialService_Process();
}
