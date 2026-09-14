#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ModbusAsciiFramer.h"
#include "ModbusLrc.h"
#include "ModbusSlave.h"

typedef struct
{
    uint16_t address;
    uint16_t value;
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
    registers->address = address;
    registers->value = value;
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
    static const uint8_t lrc_vector[] =
        {0x01U, 0x03U, 0x00U, 0x00U, 0x00U, 0x0AU};
    static const uint8_t read_pdu[] =
        {0x03U, 0x00U, 0x10U, 0x00U, 0x02U};
    static const uint8_t write_pdu[] =
        {0x06U, 0x11U, 0x03U, 0x00U, 0x0EU};
    ModbusSlaveConfig_t config;
    FakeRegisters_t registers = {0U, 0U};
    ModbusAsciiRequestView_t decoded;
    uint8_t decode_buffer[MODBUS_ASCII_MAX_BINARY_LENGTH];
    uint8_t request[MODBUS_ASCII_MAX_ADU_LENGTH];
    uint8_t response[MODBUS_ASCII_MAX_ADU_LENGTH];
    size_t request_length;
    size_t response_length;
    ModbusSlaveResult_t result;

    assert(ModbusLrc_Calculate(lrc_vector, sizeof(lrc_vector)) == 0xF2U);

    (void)memset(&config, 0, sizeof(config));
    config.unit_address = 1U;
    config.registers.read_holding_registers = ReadHolding;
    config.registers.write_single_register = WriteSingle;
    config.registers.write_multiple_registers = WriteMultiple;
    config.registers.context = &registers;

    assert(ModbusAsciiFramer_Encode(
        1U, read_pdu, sizeof(read_pdu),
        request, sizeof(request), &request_length) == MODBUS_ASCII_STATUS_OK);
    assert(request[0] == (uint8_t)':');
    assert(request[request_length - 2U] == (uint8_t)'\r');
    assert(request[request_length - 1U] == (uint8_t)'\n');

    result = ModbusSlave_ProcessAsciiRequest(
        &config, request, request_length,
        response, sizeof(response), &response_length);
    assert(result == MODBUS_SLAVE_RESULT_RESPONSE_READY);
    assert(ModbusAsciiFramer_Decode(
        response, response_length,
        decode_buffer, sizeof(decode_buffer), &decoded) == MODBUS_ASCII_STATUS_OK);
    assert(decoded.address == 1U);
    assert(decoded.pdu_length == 6U);
    assert(decoded.pdu[0] == 0x03U);
    assert(decoded.pdu[1] == 0x04U);
    assert(decoded.pdu[2] == 0x00U);
    assert(decoded.pdu[3] == 0x10U);
    assert(decoded.pdu[4] == 0x00U);
    assert(decoded.pdu[5] == 0x11U);

    assert(ModbusAsciiFramer_Encode(
        0U, write_pdu, sizeof(write_pdu),
        request, sizeof(request), &request_length) == MODBUS_ASCII_STATUS_OK);
    for (size_t index = 0U; index < request_length; index++)
    {
        if (request[index] == (uint8_t)'E')
        {
            request[index] = (uint8_t)'e';
            break;
        }
    }
    result = ModbusSlave_ProcessAsciiRequest(
        &config, request, request_length,
        response, sizeof(response), &response_length);
    assert(result == MODBUS_SLAVE_RESULT_PROCESSED_NO_RESPONSE);
    assert(response_length == 0U);
    assert(registers.address == 0x1103U);
    assert(registers.value == 0x000EU);

    request[1] = (uint8_t)'G';
    result = ModbusSlave_ProcessAsciiRequest(
        &config, request, request_length,
        response, sizeof(response), &response_length);
    assert(result == MODBUS_SLAVE_RESULT_IGNORED);

    assert(ModbusAsciiFramer_Encode(
        1U, read_pdu, sizeof(read_pdu),
        request, sizeof(request), &request_length) == MODBUS_ASCII_STATUS_OK);
    request[2] = (request[2] == (uint8_t)'0') ? (uint8_t)'1' : (uint8_t)'0';
    result = ModbusSlave_ProcessAsciiRequest(
        &config, request, request_length,
        response, sizeof(response), &response_length);
    assert(result == MODBUS_SLAVE_RESULT_LRC_ERROR);

    request[request_length - 1U] = (uint8_t)'X';
    result = ModbusSlave_ProcessAsciiRequest(
        &config, request, request_length,
        response, sizeof(response), &response_length);
    assert(result == MODBUS_SLAVE_RESULT_IGNORED);

    return 0;
}
