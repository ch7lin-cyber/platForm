#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "EventService.h"
#include "HalSerial.h"
#include "ModbusRegisterAdapter.h"
#include "SerialConfiguration.h"
#include "SerialConfigurationApplyService.h"

typedef struct
{
    HalSerialConfig_t configuration;
    uint32_t configure_count;
    bool busy;
} FakeDriver_t;

static uint32_t g_forwarded_events[HAL_SERIAL_PORT_COUNT];

static HalSerialStatus_t FakeInitialize(
    void *context,
    const HalSerialConfig_t *configuration)
{
    FakeDriver_t *driver = (FakeDriver_t *)context;
    driver->configuration = *configuration;
    driver->busy = false;
    return HAL_SERIAL_STATUS_OK;
}

static HalSerialStatus_t FakeConfigure(
    void *context,
    const HalSerialConfig_t *configuration)
{
    FakeDriver_t *driver = (FakeDriver_t *)context;
    driver->configuration = *configuration;
    driver->configure_count++;
    return HAL_SERIAL_STATUS_OK;
}

static HalSerialStatus_t FakeWrite(
    void *context,
    const uint8_t *data,
    size_t length)
{
    FakeDriver_t *driver = (FakeDriver_t *)context;
    (void)data;
    (void)length;
    driver->busy = true;
    return HAL_SERIAL_STATUS_OK;
}

static HalSerialStatus_t FakeAbort(void *context)
{
    ((FakeDriver_t *)context)->busy = false;
    return HAL_SERIAL_STATUS_OK;
}

static bool FakeIsBusy(void *context)
{
    return ((FakeDriver_t *)context)->busy;
}

static void OnForwardedEvent(
    HalSerialPort_t port,
    uint32_t event_mask,
    void *context)
{
    (void)context;
    g_forwarded_events[(uint32_t)port] |= event_mask;
}

int main(void)
{
    static const HalSerialDriverOps_t operations =
    {
        FakeInitialize,
        FakeConfigure,
        FakeWrite,
        FakeAbort,
        FakeIsBusy
    };
    FakeDriver_t drivers[HAL_SERIAL_PORT_COUNT] = {0};
    ModbusSerialPortConfiguration_t configuration;
    ModbusSerialPortConfiguration_t active;
    uint8_t response[] = {0x01U, 0x06U, 0x12U, 0x08U};
    uint16_t status;
    uint32_t index;

    assert(EventService_Initialize(EVENT_ACK_COMMUNICATION));
    for (index = 0U; index < HAL_SERIAL_PORT_COUNT; index++)
    {
        assert(HalSerial_RegisterDriver((HalSerialPort_t)index,
                                        &operations, &drivers[index]) ==
               HAL_SERIAL_STATUS_OK);
    }

    SerialConfiguration_SetRtuDefault(
        &configuration.serial, SERIAL_ROLE_MODBUS_SLAVE);
    configuration.unit_id = 1U;
    assert(SerialConfigurationApplyService_InitializePort(
        HAL_SERIAL_PORT_0, &configuration, 1000U,
        OnForwardedEvent, NULL) == SERIAL_SERVICE_STATUS_OK);

    assert(ModbusRegisterAdapter_WriteSingleRegister(
        NULL, 0x1200U, MODBUS_SERIAL_BAUD_38400) ==
           MODBUS_EXCEPTION_NONE);
    assert(ModbusRegisterAdapter_WriteSingleRegister(
        NULL, 0x1208U, MODBUS_SERIAL_APPLY_KEY) ==
           MODBUS_EXCEPTION_NONE);
    assert(drivers[0].configuration.baud_rate == 115200UL);
    assert(drivers[0].configure_count == 0U);

    assert(SerialConfigurationApplyService_WriteResponse(
        HAL_SERIAL_PORT_0, response, sizeof(response)) ==
           SERIAL_SERVICE_STATUS_OK);
    assert(drivers[0].configuration.baud_rate == 115200UL);

    drivers[0].busy = false;
    HalSerial_NotifyTransmitCompleteFromIsr(HAL_SERIAL_PORT_0);
    SerialConfigurationApplyService_Process();
    assert(drivers[0].configure_count == 1U);
    assert(drivers[0].configuration.baud_rate == 38400UL);
    assert(ModbusRegisterAdapter_GetActiveSerialConfiguration(0U, &active));
    assert(active.serial.line.baud_rate == 38400UL);
    assert((g_forwarded_events[0] & SERIAL_SERVICE_EVENT_TX_COMPLETE) != 0U);
    assert(ModbusRegisterAdapter_ReadSerialRegister(0x1209U, &status) ==
           MODBUS_EXCEPTION_NONE);
    assert(status == MODBUS_SERIAL_STATUS_ACTIVE);
    assert(ModbusRegisterAdapter_ReadSerialRegister(0x120AU, &status) ==
           MODBUS_EXCEPTION_NONE);
    assert(status == 1U);

    SerialConfiguration_SetAsciiDefault(
        &configuration.serial, SERIAL_ROLE_MODBUS_SLAVE);
    configuration.unit_id = 2U;
    assert(SerialConfigurationApplyService_InitializePort(
        HAL_SERIAL_PORT_1, &configuration, 3U,
        OnForwardedEvent, NULL) == SERIAL_SERVICE_STATUS_OK);
    assert(ModbusRegisterAdapter_WriteSingleRegister(
        NULL, 0x1210U, MODBUS_SERIAL_BAUD_9600) ==
           MODBUS_EXCEPTION_NONE);
    assert(ModbusRegisterAdapter_WriteSingleRegister(
        NULL, 0x1218U, MODBUS_SERIAL_APPLY_KEY) ==
           MODBUS_EXCEPTION_NONE);

    SerialConfigurationApplyService_Tick1ms();
    SerialConfigurationApplyService_Tick1ms();
    SerialConfigurationApplyService_Tick1ms();
    assert(!ModbusRegisterAdapter_IsApplyRequested(1U));
    assert(ModbusRegisterAdapter_ReadSerialRegister(0x1219U, &status) ==
           MODBUS_EXCEPTION_NONE);
    assert(status == MODBUS_SERIAL_STATUS_ERROR);
    assert(ModbusRegisterAdapter_GetActiveSerialConfiguration(1U, &active));
    assert(active.serial.line.baud_rate == 115200UL);

    return 0;
}
