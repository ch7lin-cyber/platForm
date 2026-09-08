#include <assert.h>
#include <stdint.h>

#include "ModbusFunction06.h"
#include "ModbusRegisterAdapter.h"

int main(void)
{
    ModbusSerialRegisterInfo_t information;
    uint32_t baud_rate;
    uint16_t baud_code;
    uint16_t address;
    ModbusSerialPortConfiguration_t active;
    ModbusSerialPortConfiguration_t applied;
    ModbusSerialPortConfiguration_t pending;
    uint8_t request[5];
    uint8_t response[5];
    size_t response_length;
    const uint16_t invalid_multiple[] =
    {
        MODBUS_SERIAL_BAUD_115200,
        9U
    };

    assert(ModbusRegisterAdapter_GetSerialAddress(
        0U, MODBUS_SERIAL_REGISTER_BAUD_CODE, &address));
    assert(address == 0x1200U);
    assert(ModbusRegisterAdapter_GetSerialAddress(
        1U, MODBUS_SERIAL_REGISTER_APPLY, &address));
    assert(address == 0x1218U);
    assert(!ModbusRegisterAdapter_GetSerialAddress(
        2U, MODBUS_SERIAL_REGISTER_BAUD_CODE, &address));

    assert(ModbusRegisterAdapter_ResolveSerialAddress(0x1200U,
                                                       &information));
    assert(information.port == 0U);
    assert(information.field == MODBUS_SERIAL_REGISTER_BAUD_CODE);
    assert(information.access == MODBUS_REGISTER_ACCESS_READ_WRITE);

    assert(ModbusRegisterAdapter_ResolveSerialAddress(0x1218U,
                                                       &information));
    assert(information.port == 1U);
    assert(information.field == MODBUS_SERIAL_REGISTER_APPLY);
    assert(information.access == MODBUS_REGISTER_ACCESS_WRITE_ONLY);
    assert(!ModbusRegisterAdapter_ResolveSerialAddress(0x120BU,
                                                        &information));
    assert(!ModbusRegisterAdapter_ResolveSerialAddress(0x1220U,
                                                        &information));

    assert(ModbusRegisterAdapter_BaudCodeToRate(
        MODBUS_SERIAL_BAUD_115200, &baud_rate));
    assert(baud_rate == 115200UL);
    assert(!ModbusRegisterAdapter_BaudCodeToRate(6U, &baud_rate));
    assert(ModbusRegisterAdapter_BaudRateToCode(38400UL, &baud_code));
    assert(baud_code == MODBUS_SERIAL_BAUD_38400);
    assert(!ModbusRegisterAdapter_BaudRateToCode(12345UL, &baud_code));

    assert(ModbusRegisterAdapter_IsSerialValueValid(
        MODBUS_SERIAL_REGISTER_DATA_BITS, 7U));
    assert(!ModbusRegisterAdapter_IsSerialValueValid(
        MODBUS_SERIAL_REGISTER_DATA_BITS, 9U));
    assert(ModbusRegisterAdapter_IsSerialValueValid(
        MODBUS_SERIAL_REGISTER_UNIT_ID, 247U));
    assert(!ModbusRegisterAdapter_IsSerialValueValid(
        MODBUS_SERIAL_REGISTER_UNIT_ID, 0U));
    assert(ModbusRegisterAdapter_IsSerialValueValid(
        MODBUS_SERIAL_REGISTER_APPLY, MODBUS_SERIAL_APPLY_KEY));
    assert(!ModbusRegisterAdapter_IsSerialValueValid(
        MODBUS_SERIAL_REGISTER_STATUS, 0U));

    SerialConfiguration_SetRtuDefault(&active.serial,
                                      SERIAL_ROLE_MODBUS_SLAVE);
    active.unit_id = 4U;
    assert(ModbusRegisterAdapter_InitializeSerialPort(0U, &active));

    request[0] = MODBUS_FUNCTION_WRITE_SINGLE_REGISTER;
    ModbusPdu_WriteU16(&request[1], 0x1200U);
    ModbusPdu_WriteU16(&request[3], MODBUS_SERIAL_BAUD_38400);
    assert(ModbusFunction06_Process(
        request, sizeof(request),
        ModbusRegisterAdapter_WriteSingleRegister, NULL,
        response, sizeof(response), &response_length) == MODBUS_PDU_STATUS_OK);
    assert(response_length == sizeof(request));
    assert(ModbusRegisterAdapter_GetPendingSerialConfiguration(0U, &pending));
    assert(pending.serial.line.baud_rate == 38400UL);
    assert(ModbusRegisterAdapter_GetActiveSerialConfiguration(0U, &active));
    assert(active.serial.line.baud_rate == 115200UL);
    assert(ModbusRegisterAdapter_ReadSerialRegister(0x1209U, &baud_code) ==
           MODBUS_EXCEPTION_NONE);
    assert(baud_code == MODBUS_SERIAL_STATUS_PENDING);

    ModbusPdu_WriteU16(&request[1], 0x1208U);
    ModbusPdu_WriteU16(&request[3], MODBUS_SERIAL_APPLY_KEY);
    assert(ModbusFunction06_Process(
        request, sizeof(request),
        ModbusRegisterAdapter_WriteSingleRegister, NULL,
        response, sizeof(response), &response_length) == MODBUS_PDU_STATUS_OK);
    assert(ModbusRegisterAdapter_IsApplyRequested(0U));
    assert(ModbusRegisterAdapter_ReadSerialRegister(0x1209U, &baud_code) ==
           MODBUS_EXCEPTION_NONE);
    assert(baud_code == MODBUS_SERIAL_STATUS_WAITING_TX_COMPLETE);
    assert(ModbusRegisterAdapter_WriteSingleRegister(
        NULL, 0x1207U, 2000U) == MODBUS_EXCEPTION_SERVER_DEVICE_BUSY);

    assert(ModbusRegisterAdapter_BeginApply(0U, &applied));
    assert(applied.serial.line.baud_rate == 38400UL);
    assert(ModbusRegisterAdapter_CompleteApply(0U, true));
    assert(ModbusRegisterAdapter_GetActiveSerialConfiguration(0U, &active));
    assert(active.serial.line.baud_rate == 38400UL);
    assert(ModbusRegisterAdapter_ReadSerialRegister(0x120AU, &baud_code) ==
           MODBUS_EXCEPTION_NONE);
    assert(baud_code == 1U);

    assert(ModbusRegisterAdapter_WriteMultipleRegisters(
        NULL, 0x1200U, invalid_multiple, 2U) ==
           MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
    assert(ModbusRegisterAdapter_GetPendingSerialConfiguration(0U, &pending));
    assert(pending.serial.line.baud_rate == 38400UL);
    assert(pending.serial.line.data_bits == HAL_SERIAL_DATA_BITS_8);

    assert(ModbusRegisterAdapter_WriteSingleRegister(
        NULL, 0x1201U, 7U) == MODBUS_EXCEPTION_NONE);
    assert(ModbusRegisterAdapter_WriteSingleRegister(
        NULL, 0x1208U, MODBUS_SERIAL_APPLY_KEY) ==
           MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
    assert(ModbusRegisterAdapter_DiscardPending(0U));
    assert(ModbusRegisterAdapter_GetPendingSerialConfiguration(0U, &pending));
    assert(pending.serial.line.data_bits == HAL_SERIAL_DATA_BITS_8);

    return 0;
}
