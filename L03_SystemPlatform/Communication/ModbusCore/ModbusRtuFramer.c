#include "ModbusRtuFramer.h"

#include <stddef.h>
#include <string.h>

#include "ModbusCrc16.h"

ModbusRtuStatus_t ModbusRtuFramer_Decode(
    const uint8_t *adu,
    size_t adu_length,
    ModbusRtuRequestView_t *request)
{
    if ((adu == NULL) || (request == NULL))
    {
        return MODBUS_RTU_STATUS_INVALID_ARGUMENT;
    }

    request->address = 0U;
    request->pdu = NULL;
    request->pdu_length = 0U;

    if (adu_length < MODBUS_RTU_MIN_ADU_LENGTH)
    {
        return MODBUS_RTU_STATUS_FRAME_TOO_SHORT;
    }
    if (adu_length > MODBUS_RTU_MAX_ADU_LENGTH)
    {
        return MODBUS_RTU_STATUS_FRAME_TOO_LONG;
    }
    if (!ModbusCrc16_IsFrameValid(adu, adu_length))
    {
        return MODBUS_RTU_STATUS_CRC_ERROR;
    }

    request->address = adu[0];
    request->pdu = &adu[1];
    request->pdu_length = adu_length - 3U;
    return MODBUS_RTU_STATUS_OK;
}

ModbusRtuStatus_t ModbusRtuFramer_Encode(
    uint8_t address,
    const uint8_t *pdu,
    size_t pdu_length,
    uint8_t *adu,
    size_t adu_capacity,
    size_t *adu_length)
{
    size_t required_length;

    if ((pdu == NULL) || (adu == NULL) || (adu_length == NULL) ||
        (pdu_length == 0U))
    {
        return MODBUS_RTU_STATUS_INVALID_ARGUMENT;
    }

    *adu_length = 0U;
    if (pdu_length > MODBUS_RTU_MAX_PDU_LENGTH)
    {
        return MODBUS_RTU_STATUS_FRAME_TOO_LONG;
    }

    required_length = pdu_length + 3U;
    if (adu_capacity < required_length)
    {
        return MODBUS_RTU_STATUS_RESPONSE_TOO_SMALL;
    }

    adu[0] = address;
    (void)memcpy(&adu[1], pdu, pdu_length);
    if (!ModbusCrc16_Append(adu, pdu_length + 1U, adu_capacity))
    {
        return MODBUS_RTU_STATUS_RESPONSE_TOO_SMALL;
    }

    *adu_length = required_length;
    return MODBUS_RTU_STATUS_OK;
}
