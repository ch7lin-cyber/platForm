#include <assert.h>
#include <stdint.h>

#include "ModbusRegisterAdapter.h"

int main(void)
{
    ModbusSerialRegisterInfo_t information;
    uint32_t baud_rate;
    uint16_t baud_code;
    uint16_t address;

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

    return 0;
}
