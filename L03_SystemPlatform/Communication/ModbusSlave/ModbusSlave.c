#include "ModbusSlave.h"

#include <stddef.h>

#include "ModbusFunction04.h"
#include "ModbusPdu.h"
#include "ModbusRtuFramer.h"

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
    ModbusPduStatus_t pdu_status;
    uint8_t response_pdu[MODBUS_RTU_MAX_PDU_LENGTH];
    size_t response_pdu_length = 0U;
    bool is_broadcast;

    if ((config == NULL) || (request_adu == NULL) ||
        (response_adu == NULL) || (response_length == NULL) ||
        (config->unit_address < MODBUS_SLAVE_ADDRESS_MIN) ||
        (config->unit_address > MODBUS_SLAVE_ADDRESS_MAX))
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

    is_broadcast = (request.address == MODBUS_BROADCAST_ADDRESS);
    if ((!is_broadcast) && (request.address != config->unit_address))
    {
        return MODBUS_SLAVE_RESULT_IGNORED;
    }

    if (is_broadcast &&
        (request.pdu[0] != MODBUS_FUNCTION_WRITE_SINGLE_REGISTER) &&
        (request.pdu[0] != MODBUS_FUNCTION_WRITE_MULTIPLE_REGISTERS))
    {
        return MODBUS_SLAVE_RESULT_IGNORED;
    }

    pdu_status = ProcessPdu(
        config, request.pdu, request.pdu_length,
        response_pdu, sizeof(response_pdu), &response_pdu_length);
    if (pdu_status == MODBUS_PDU_STATUS_RESPONSE_TOO_SMALL)
    {
        return MODBUS_SLAVE_RESULT_RESPONSE_TOO_SMALL;
    }
    if (pdu_status != MODBUS_PDU_STATUS_OK)
    {
        return MODBUS_SLAVE_RESULT_INVALID_ARGUMENT;
    }

    if (is_broadcast)
    {
        return MODBUS_SLAVE_RESULT_PROCESSED_NO_RESPONSE;
    }

    rtu_status = ModbusRtuFramer_Encode(
        config->unit_address,
        response_pdu,
        response_pdu_length,
        response_adu,
        response_capacity,
        response_length);
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
