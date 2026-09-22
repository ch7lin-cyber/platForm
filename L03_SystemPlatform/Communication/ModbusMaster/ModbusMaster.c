#include "ModbusMaster.h"

#include <stdbool.h>

#include "ModbusAsciiFramer.h"
#include "ModbusRtuFramer.h"

#define MODBUS_FUNCTION_READ_HOLDING_REGISTERS  (0x03U)
#define MODBUS_FUNCTION_READ_INPUT_REGISTERS    (0x04U)
#define MODBUS_FUNCTION_WRITE_SINGLE_REGISTER  (0x06U)
#define MODBUS_FUNCTION_WRITE_MULTIPLE_REGISTERS (0x10U)

static bool IsReadFunction(uint8_t function_code)
{
    return ((function_code == MODBUS_FUNCTION_READ_HOLDING_REGISTERS) ||
            (function_code == MODBUS_FUNCTION_READ_INPUT_REGISTERS));
}

static bool IsRequestValid(const ModbusMasterRequest_t *request)
{
    if ((request == NULL) || (request->unit_address == 0U) ||
        (request->unit_address > 247U) || (request->quantity == 0U))
    {
        return false;
    }
    if (IsReadFunction(request->function_code))
    {
        return (request->quantity <= MODBUS_MASTER_READ_QUANTITY_MAX);
    }
    if (request->function_code == MODBUS_FUNCTION_WRITE_SINGLE_REGISTER)
    {
        return ((request->quantity == 1U) &&
                (request->write_values != NULL));
    }
    if (request->function_code == MODBUS_FUNCTION_WRITE_MULTIPLE_REGISTERS)
    {
        return ((request->quantity <= MODBUS_MASTER_WRITE_QUANTITY_MAX) &&
                (request->write_values != NULL));
    }
    return false;
}

static ModbusMasterStatus_t BuildPdu(
    const ModbusMasterRequest_t *request,
    uint8_t *pdu,
    size_t capacity,
    size_t *length)
{
    uint16_t index;

    if ((!IsRequestValid(request)) || (pdu == NULL) || (length == NULL))
    {
        return MODBUS_MASTER_STATUS_INVALID_ARGUMENT;
    }

    if (IsReadFunction(request->function_code) ||
        (request->function_code == MODBUS_FUNCTION_WRITE_SINGLE_REGISTER))
    {
        if (capacity < 5U)
        {
            return MODBUS_MASTER_STATUS_BUFFER_TOO_SMALL;
        }
        pdu[0] = request->function_code;
        ModbusPdu_WriteU16(&pdu[1], request->starting_address);
        ModbusPdu_WriteU16(
            &pdu[3],
            (request->function_code == MODBUS_FUNCTION_WRITE_SINGLE_REGISTER) ?
                request->write_values[0] : request->quantity);
        *length = 5U;
        return MODBUS_MASTER_STATUS_OK;
    }

    if (capacity < (size_t)(6U + (2U * request->quantity)))
    {
        return MODBUS_MASTER_STATUS_BUFFER_TOO_SMALL;
    }
    pdu[0] = request->function_code;
    ModbusPdu_WriteU16(&pdu[1], request->starting_address);
    ModbusPdu_WriteU16(&pdu[3], request->quantity);
    pdu[5] = (uint8_t)(2U * request->quantity);
    for (index = 0U; index < request->quantity; index++)
    {
        ModbusPdu_WriteU16(&pdu[6U + (2U * index)],
                           request->write_values[index]);
    }
    *length = (size_t)(6U + (2U * request->quantity));
    return MODBUS_MASTER_STATUS_OK;
}

ModbusMasterStatus_t ModbusMaster_BuildRequest(
    SerialProtocol_t protocol,
    const ModbusMasterRequest_t *request,
    uint8_t *adu,
    size_t adu_capacity,
    size_t *adu_length)
{
    uint8_t pdu[MODBUS_RTU_MAX_PDU_LENGTH];
    size_t pdu_length;
    ModbusMasterStatus_t status;

    status = BuildPdu(request, pdu, sizeof(pdu), &pdu_length);
    if (status != MODBUS_MASTER_STATUS_OK)
    {
        return status;
    }
    if (protocol == SERIAL_PROTOCOL_MODBUS_RTU)
    {
        ModbusRtuStatus_t rtu_status = ModbusRtuFramer_Encode(
            request->unit_address, pdu, pdu_length,
            adu, adu_capacity, adu_length);
        return (rtu_status == MODBUS_RTU_STATUS_OK) ?
            MODBUS_MASTER_STATUS_OK : MODBUS_MASTER_STATUS_BUFFER_TOO_SMALL;
    }
    if (protocol == SERIAL_PROTOCOL_MODBUS_ASCII)
    {
        ModbusAsciiStatus_t ascii_status = ModbusAsciiFramer_Encode(
            request->unit_address, pdu, pdu_length,
            adu, adu_capacity, adu_length);
        return (ascii_status == MODBUS_ASCII_STATUS_OK) ?
            MODBUS_MASTER_STATUS_OK : MODBUS_MASTER_STATUS_BUFFER_TOO_SMALL;
    }
    return MODBUS_MASTER_STATUS_INVALID_ARGUMENT;
}

static ModbusMasterStatus_t ValidateResponsePdu(
    const ModbusMasterRequest_t *request,
    const uint8_t *pdu,
    size_t pdu_length,
    uint16_t *read_values,
    size_t read_capacity,
    size_t *read_count,
    ModbusExceptionCode_t *exception)
{
    uint16_t index;

    *read_count = 0U;
    *exception = MODBUS_EXCEPTION_NONE;
    if ((pdu == NULL) || (pdu_length < 2U))
    {
        return MODBUS_MASTER_STATUS_FRAME_ERROR;
    }
    if (pdu[0] == (uint8_t)(request->function_code | 0x80U))
    {
        *exception = (ModbusExceptionCode_t)pdu[1];
        return MODBUS_MASTER_STATUS_EXCEPTION_RESPONSE;
    }
    if (pdu[0] != request->function_code)
    {
        return MODBUS_MASTER_STATUS_FUNCTION_MISMATCH;
    }

    if (IsReadFunction(request->function_code))
    {
        if ((pdu_length != (size_t)(2U + pdu[1])) ||
            (pdu[1] != (uint8_t)(request->quantity * 2U)) ||
            (read_values == NULL) || (read_capacity < request->quantity))
        {
            return MODBUS_MASTER_STATUS_DATA_MISMATCH;
        }
        for (index = 0U; index < request->quantity; index++)
        {
            read_values[index] = ModbusPdu_ReadU16(&pdu[2U + (2U * index)]);
        }
        *read_count = request->quantity;
        return MODBUS_MASTER_STATUS_OK;
    }

    if (pdu_length != 5U)
    {
        return MODBUS_MASTER_STATUS_DATA_MISMATCH;
    }
    if ((ModbusPdu_ReadU16(&pdu[1]) != request->starting_address) ||
        (ModbusPdu_ReadU16(&pdu[3]) !=
         ((request->function_code == MODBUS_FUNCTION_WRITE_SINGLE_REGISTER) ?
             request->write_values[0] : request->quantity)))
    {
        return MODBUS_MASTER_STATUS_DATA_MISMATCH;
    }
    return MODBUS_MASTER_STATUS_OK;
}

ModbusMasterStatus_t ModbusMaster_ParseResponse(
    SerialProtocol_t protocol,
    const ModbusMasterRequest_t *request,
    const uint8_t *adu,
    size_t adu_length,
    uint16_t *read_values,
    size_t read_capacity,
    size_t *read_count,
    ModbusExceptionCode_t *exception)
{
    uint8_t decoded[MODBUS_ASCII_MAX_BINARY_LENGTH];
    uint8_t address;
    const uint8_t *pdu;
    size_t pdu_length;

    if ((!IsRequestValid(request)) || (adu == NULL) ||
        (read_count == NULL) || (exception == NULL))
    {
        return MODBUS_MASTER_STATUS_INVALID_ARGUMENT;
    }

    if (protocol == SERIAL_PROTOCOL_MODBUS_RTU)
    {
        ModbusRtuRequestView_t view;
        if (ModbusRtuFramer_Decode(adu, adu_length, &view) !=
            MODBUS_RTU_STATUS_OK)
        {
            return MODBUS_MASTER_STATUS_FRAME_ERROR;
        }
        address = view.address;
        pdu = view.pdu;
        pdu_length = view.pdu_length;
    }
    else if (protocol == SERIAL_PROTOCOL_MODBUS_ASCII)
    {
        ModbusAsciiRequestView_t view;
        if (ModbusAsciiFramer_Decode(
                adu, adu_length, decoded, sizeof(decoded), &view) !=
            MODBUS_ASCII_STATUS_OK)
        {
            return MODBUS_MASTER_STATUS_FRAME_ERROR;
        }
        address = view.address;
        pdu = view.pdu;
        pdu_length = view.pdu_length;
    }
    else
    {
        return MODBUS_MASTER_STATUS_INVALID_ARGUMENT;
    }

    if (address != request->unit_address)
    {
        return MODBUS_MASTER_STATUS_ADDRESS_MISMATCH;
    }
    return ValidateResponsePdu(
        request, pdu, pdu_length,
        read_values, read_capacity, read_count, exception);
}
