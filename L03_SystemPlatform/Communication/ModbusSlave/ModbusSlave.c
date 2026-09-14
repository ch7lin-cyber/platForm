#include "ModbusSlave.h"

#include <stdbool.h>
#include <stddef.h>

#include "ModbusAsciiFramer.h"
#include "ModbusFunction04.h"
#include "ModbusPdu.h"
#include "ModbusRtuFramer.h"

static bool IsConfigValid(const ModbusSlaveConfig_t *config)
{
    return ((config != NULL) &&
            (config->unit_address >= MODBUS_SLAVE_ADDRESS_MIN) &&
            (config->unit_address <= MODBUS_SLAVE_ADDRESS_MAX));
}

static bool IsBroadcastWrite(uint8_t address, const uint8_t *pdu, size_t pdu_length)
{
    if ((address != MODBUS_BROADCAST_ADDRESS) ||
        (pdu == NULL) || (pdu_length == 0U))
    {
        return false;
    }

    return ((pdu[0] == MODBUS_FUNCTION_WRITE_SINGLE_REGISTER) ||
            (pdu[0] == MODBUS_FUNCTION_WRITE_MULTIPLE_REGISTERS));
}

static ModbusPduStatus_t BuildIllegalFunction(
    uint8_t function_code,
    uint8_t *response_pdu,
    size_t response_capacity,
    size_t *response_length)
{
    return ModbusPdu_BuildException(
        function_code,
        MODBUS_EXCEPTION_ILLEGAL_FUNCTION,
        response_pdu,
        response_capacity,
        response_length);
}

static ModbusPduStatus_t ProcessPdu(
    const ModbusSlaveConfig_t *config,
    const uint8_t *request_pdu,
    size_t request_length,
    uint8_t *response_pdu,
    size_t response_capacity,
    size_t *response_length)
{
    uint8_t function_code;

    if ((request_pdu == NULL) || (request_length == 0U))
    {
        return MODBUS_PDU_STATUS_INVALID_ARGUMENT;
    }

    function_code = request_pdu[0];
    switch (function_code)
    {
        case MODBUS_FUNCTION_READ_HOLDING_REGISTERS:
            if (config->registers.read_holding_registers == NULL)
            {
                return BuildIllegalFunction(function_code, response_pdu,
                                            response_capacity, response_length);
            }
            return ModbusFunction03_Process(
                request_pdu, request_length,
                config->registers.read_holding_registers,
                config->registers.context,
                response_pdu, response_capacity, response_length);

        case MODBUS_FUNCTION_READ_INPUT_REGISTERS:
            if (config->registers.read_input_registers == NULL)
            {
                return BuildIllegalFunction(function_code, response_pdu,
                                            response_capacity, response_length);
            }
            return ModbusFunction04_Process(
                request_pdu, request_length,
                config->registers.read_input_registers,
                config->registers.context,
                response_pdu, response_capacity, response_length);

        case MODBUS_FUNCTION_WRITE_SINGLE_REGISTER:
            if (config->registers.write_single_register == NULL)
            {
                return BuildIllegalFunction(function_code, response_pdu,
                                            response_capacity, response_length);
            }
            return ModbusFunction06_Process(
                request_pdu, request_length,
                config->registers.write_single_register,
                config->registers.context,
                response_pdu, response_capacity, response_length);

        case MODBUS_FUNCTION_WRITE_MULTIPLE_REGISTERS:
            if (config->registers.write_multiple_registers == NULL)
            {
                return BuildIllegalFunction(function_code, response_pdu,
                                            response_capacity, response_length);
            }
            return ModbusFunction10_Process(
                request_pdu, request_length,
                config->registers.write_multiple_registers,
                config->registers.context,
                response_pdu, response_capacity, response_length);

        default:
            return BuildIllegalFunction(function_code, response_pdu,
                                        response_capacity, response_length);
    }
}

static ModbusSlaveResult_t ProcessAddressedPdu(
    const ModbusSlaveConfig_t *config,
    uint8_t address,
    const uint8_t *request_pdu,
    size_t request_pdu_length,
    uint8_t *response_pdu,
    size_t response_pdu_capacity,
    size_t *response_pdu_length)
{
    ModbusPduStatus_t pdu_status;
    bool is_broadcast;

    is_broadcast = (address == MODBUS_BROADCAST_ADDRESS);
    if ((!is_broadcast) && (address != config->unit_address))
    {
        return MODBUS_SLAVE_RESULT_IGNORED;
    }
    if (is_broadcast &&
        (!IsBroadcastWrite(address, request_pdu, request_pdu_length)))
    {
        return MODBUS_SLAVE_RESULT_IGNORED;
    }

    pdu_status = ProcessPdu(
        config, request_pdu, request_pdu_length,
        response_pdu, response_pdu_capacity, response_pdu_length);
    if (pdu_status == MODBUS_PDU_STATUS_RESPONSE_TOO_SMALL)
    {
        return MODBUS_SLAVE_RESULT_RESPONSE_TOO_SMALL;
    }
    if (pdu_status != MODBUS_PDU_STATUS_OK)
    {
        return MODBUS_SLAVE_RESULT_INVALID_ARGUMENT;
    }

    return is_broadcast ? MODBUS_SLAVE_RESULT_PROCESSED_NO_RESPONSE :
                          MODBUS_SLAVE_RESULT_RESPONSE_READY;
}

ModbusSlaveResult_t ModbusSlave_ProcessRtuRequest(
    const ModbusSlaveConfig_t *config,
    const uint8_t *request_adu,
    size_t request_length,
    uint8_t *response_adu,
    size_t response_capacity,
    size_t *response_length)
{
    ModbusRtuRequestView_t request;
    ModbusRtuStatus_t rtu_status;
    ModbusSlaveResult_t result;
    uint8_t response_pdu[MODBUS_RTU_MAX_PDU_LENGTH];
    size_t response_pdu_length = 0U;

    if ((!IsConfigValid(config)) || (request_adu == NULL) ||
        (response_adu == NULL) || (response_length == NULL))
    {
        return MODBUS_SLAVE_RESULT_INVALID_ARGUMENT;
    }

    *response_length = 0U;
    rtu_status = ModbusRtuFramer_Decode(request_adu, request_length, &request);
    if (rtu_status == MODBUS_RTU_STATUS_CRC_ERROR)
    {
        return MODBUS_SLAVE_RESULT_CRC_ERROR;
    }
    if (rtu_status != MODBUS_RTU_STATUS_OK)
    {
        return MODBUS_SLAVE_RESULT_IGNORED;
    }

    result = ProcessAddressedPdu(
        config, request.address, request.pdu, request.pdu_length,
        response_pdu, sizeof(response_pdu), &response_pdu_length);
    if (result != MODBUS_SLAVE_RESULT_RESPONSE_READY)
    {
        return result;
    }

    rtu_status = ModbusRtuFramer_Encode(
        config->unit_address, response_pdu, response_pdu_length,
        response_adu, response_capacity, response_length);
    if (rtu_status == MODBUS_RTU_STATUS_RESPONSE_TOO_SMALL)
    {
        return MODBUS_SLAVE_RESULT_RESPONSE_TOO_SMALL;
    }
    if (rtu_status != MODBUS_RTU_STATUS_OK)
    {
        return MODBUS_SLAVE_RESULT_INVALID_ARGUMENT;
    }

    return MODBUS_SLAVE_RESULT_RESPONSE_READY;
}

ModbusSlaveResult_t ModbusSlave_ProcessAsciiRequest(
    const ModbusSlaveConfig_t *config,
    const uint8_t *request_adu,
    size_t request_length,
    uint8_t *response_adu,
    size_t response_capacity,
    size_t *response_length)
{
    ModbusAsciiRequestView_t request;
    ModbusAsciiStatus_t ascii_status;
    ModbusSlaveResult_t result;
    uint8_t decoded_request[MODBUS_ASCII_MAX_BINARY_LENGTH];
    uint8_t response_pdu[MODBUS_ASCII_MAX_PDU_LENGTH];
    size_t response_pdu_length = 0U;

    if ((!IsConfigValid(config)) || (request_adu == NULL) ||
        (response_adu == NULL) || (response_length == NULL))
    {
        return MODBUS_SLAVE_RESULT_INVALID_ARGUMENT;
    }

    *response_length = 0U;
    ascii_status = ModbusAsciiFramer_Decode(
        request_adu, request_length,
        decoded_request, sizeof(decoded_request), &request);
    if (ascii_status == MODBUS_ASCII_STATUS_LRC_ERROR)
    {
        return MODBUS_SLAVE_RESULT_LRC_ERROR;
    }
    if (ascii_status != MODBUS_ASCII_STATUS_OK)
    {
        return MODBUS_SLAVE_RESULT_IGNORED;
    }

    result = ProcessAddressedPdu(
        config, request.address, request.pdu, request.pdu_length,
        response_pdu, sizeof(response_pdu), &response_pdu_length);
    if (result != MODBUS_SLAVE_RESULT_RESPONSE_READY)
    {
        return result;
    }

    ascii_status = ModbusAsciiFramer_Encode(
        config->unit_address, response_pdu, response_pdu_length,
        response_adu, response_capacity, response_length);
    if (ascii_status == MODBUS_ASCII_STATUS_BUFFER_TOO_SMALL)
    {
        return MODBUS_SLAVE_RESULT_RESPONSE_TOO_SMALL;
    }
    if (ascii_status != MODBUS_ASCII_STATUS_OK)
    {
        return MODBUS_SLAVE_RESULT_INVALID_ARGUMENT;
    }

    return MODBUS_SLAVE_RESULT_RESPONSE_READY;
}
