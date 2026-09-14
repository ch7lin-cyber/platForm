#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ModbusCrc16.h"
#include "ModbusRtuFramer.h"
#include "ModbusSlave.h"

typedef struct
{
    uint16_t write_address;
    uint16_t write_value;
} FakeRegisters_t;

static ModbusExceptionCode_t ReadHolding(
    void *context, uint16_t address, uint16_t quantity, uint16_t *values)
{
    uint16_t index;
    (void)context;
    for (index = 0U; index < quantity; index++)
    {
        values[index] = (uint16_t)(address + index);
    }
    return MODBUS_EXCEPTION_NONE;
}

static ModbusExceptionCode_t WriteSingle(
    void *context, uint16_t address, uint16_t value)
{
    FakeRegisters_t *registers = (FakeRegisters_t *)context;
    registers->write_address = address;
    registers->write_value = value;
    return MODBUS_EXCEPTION_NONE;
}

static ModbusExceptionCode_t WriteMultiple(
    void *context, uint16_t address, const uint16_t *values, uint16_t quantity)
{
    (void)context;
    (void)address;
    (void)values;
    (void)quantity;
    return MODBUS_EXCEPTION_NONE;
}

int main(void)
{
    static const uint8_t crc_vector[] =
        {0x01U, 0x03U, 0x00U, 0x00U, 0x00U, 0x0AU};
    ModbusSlaveConfig_t config;
    FakeRegisters_t registers = {0U, 0U};
    uint8_t request[16];
    uint8_t response[MODBUS_RTU_MAX_ADU_LENGTH];
    size_t response_length;
    ModbusSlaveResult_t result;

    assert(ModbusCrc16_Calculate(crc_vector, sizeof(crc_vector)) == 0xCDC5U);

    (void)memset(&config, 0, sizeof(config));
    config.unit_address = 1U;
    config.registers.read_holding_registers = ReadHolding;
    config.registers.write_single_register = WriteSingle;
    config.registers.write_multiple_registers = WriteMultiple;
    config.registers.context = &registers;

    request[0] = 0x01U;
    request[1] = 0x03U;
    request[2] = 0x00U;
    request[3] = 0x10U;
    request[4] = 0x00U;
    request[5] = 0x02U;
    assert(ModbusCrc16_Append(request, 6U, sizeof(request)));

    result = ModbusSlave_ProcessRtuRequest(
        &config, request, 8U, response, sizeof(response), &response_length);
    assert(result == MODBUS_SLAVE_RESULT_RESPONSE_READY);
    assert(response_length == 9U);
    assert(response[0] == 0x01U);
    assert(response[1] == 0x03U);
    assert(response[2] == 0x04U);
    assert(response[3] == 0x00U);
    assert(response[4] == 0x10U);
    assert(response[5] == 0x00U);
    assert(response[6] == 0x11U);
    assert(ModbusCrc16_IsFrameValid(response, response_length));

    request[0] = 0x00U;
    request[1] = 0x06U;
    request[2] = 0x11U;
    request[3] = 0x03U;
    request[4] = 0x00U;
    request[5] = 0x0EU;
    assert(ModbusCrc16_Append(request, 6U, sizeof(request)));
    result = ModbusSlave_ProcessRtuRequest(
        &config, request, 8U, response, sizeof(response), &response_length);
    assert(result == MODBUS_SLAVE_RESULT_PROCESSED_NO_RESPONSE);
    assert(response_length == 0U);
    assert(registers.write_address == 0x1103U);
    assert(registers.write_value == 0x000EU);

    request[7] ^= 0x01U;
    result = ModbusSlave_ProcessRtuRequest(
        &config, request, 8U, response, sizeof(response), &response_length);
    assert(result == MODBUS_SLAVE_RESULT_CRC_ERROR);
    assert(response_length == 0U);

    return 0;
}
