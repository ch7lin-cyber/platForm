#include "ModbusFunction03.h"

#include <stdint.h>

static ModbusExceptionCode_t NormalizeException(ModbusExceptionCode_t exception)
{
    switch (exception)
    {
        case MODBUS_EXCEPTION_NONE:
        case MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS:
        case MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE:
        case MODBUS_EXCEPTION_SERVER_DEVICE_FAILURE:
        case MODBUS_EXCEPTION_SERVER_DEVICE_BUSY:
            return exception;
        default:
            return MODBUS_EXCEPTION_SERVER_DEVICE_FAILURE;
    }
}

ModbusPduStatus_t ModbusFunction03_Process(
    const uint8_t *request_pdu,
    size_t request_length,
    ModbusReadRegistersCallback_t read_registers,
    void *context,
    uint8_t *response_pdu,
    size_t response_capacity,
    size_t *response_length)
{
    uint16_t values[MODBUS_FUNCTION_READ_MAX_REGISTERS];
    uint16_t starting_address;
    uint16_t quantity;
    uint16_t index;
    size_t required_length;
    ModbusExceptionCode_t exception;

    if ((request_pdu == NULL) || (read_registers == NULL) ||
        (response_pdu == NULL) || (response_length == NULL))
    {
        return MODBUS_PDU_STATUS_INVALID_ARGUMENT;
    }

    *response_length = 0U;
    if ((request_length != MODBUS_FUNCTION_READ_REQUEST_LENGTH) ||
        (request_pdu[0] != MODBUS_FUNCTION_READ_HOLDING_REGISTERS))
    {
        return ModbusPdu_BuildException(
            MODBUS_FUNCTION_READ_HOLDING_REGISTERS,
            MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE,
            response_pdu,
            response_capacity,
            response_length);
    }

    starting_address = ModbusPdu_ReadU16(&request_pdu[1]);
    quantity = ModbusPdu_ReadU16(&request_pdu[3]);
    if ((quantity == 0U) || (quantity > MODBUS_FUNCTION_READ_MAX_REGISTERS))
    {
        return ModbusPdu_BuildException(
            MODBUS_FUNCTION_READ_HOLDING_REGISTERS,
            MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE,
            response_pdu,
            response_capacity,
            response_length);
    }
    if (((uint32_t)starting_address + (uint32_t)quantity - 1UL) > UINT16_MAX)
    {
        return ModbusPdu_BuildException(
            MODBUS_FUNCTION_READ_HOLDING_REGISTERS,
            MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS,
            response_pdu,
            response_capacity,
            response_length);
    }

    required_length = 2U + ((size_t)quantity * 2U);
    if (response_capacity < required_length)
    {
        return MODBUS_PDU_STATUS_RESPONSE_TOO_SMALL;
    }

    exception = NormalizeException(
        read_registers(context, starting_address, quantity, values));
    if (exception != MODBUS_EXCEPTION_NONE)
    {
        return ModbusPdu_BuildException(
            MODBUS_FUNCTION_READ_HOLDING_REGISTERS,
            exception,
            response_pdu,
            response_capacity,
            response_length);
    }

    response_pdu[0] = MODBUS_FUNCTION_READ_HOLDING_REGISTERS;
    response_pdu[1] = (uint8_t)(quantity * 2U);
    for (index = 0U; index < quantity; index++)
    {
        ModbusPdu_WriteU16(&response_pdu[2U + ((size_t)index * 2U)], values[index]);
    }

    *response_length = required_length;
    return MODBUS_PDU_STATUS_OK;
}
