#include "ModbusAsciiFramer.h"

#include <stdbool.h>
#include <stddef.h>

#include "ModbusLrc.h"

static bool HexToNibble(uint8_t character, uint8_t *nibble)
{
    if ((character >= (uint8_t)'0') && (character <= (uint8_t)'9'))
    {
        *nibble = (uint8_t)(character - (uint8_t)'0');
        return true;
    }
    if ((character >= (uint8_t)'A') && (character <= (uint8_t)'F'))
    {
        *nibble = (uint8_t)(character - (uint8_t)'A' + 10U);
        return true;
    }
    if ((character >= (uint8_t)'a') && (character <= (uint8_t)'f'))
    {
        *nibble = (uint8_t)(character - (uint8_t)'a' + 10U);
        return true;
    }

    return false;
}

static uint8_t NibbleToHex(uint8_t nibble)
{
    if (nibble < 10U)
    {
        return (uint8_t)((uint8_t)'0' + nibble);
    }

    return (uint8_t)((uint8_t)'A' + (nibble - 10U));
}

static void EncodeByte(uint8_t value, uint8_t *output)
{
    output[0] = NibbleToHex((uint8_t)(value >> 4U));
    output[1] = NibbleToHex((uint8_t)(value & 0x0FU));
}

ModbusAsciiStatus_t ModbusAsciiFramer_Decode(
    const uint8_t *adu,
    size_t adu_length,
    uint8_t *decode_buffer,
    size_t decode_capacity,
    ModbusAsciiRequestView_t *request)
{
    size_t hex_length;
    size_t decoded_length;
    size_t index;
    uint8_t high_nibble;
    uint8_t low_nibble;

    if ((adu == NULL) || (decode_buffer == NULL) || (request == NULL))
    {
        return MODBUS_ASCII_STATUS_INVALID_ARGUMENT;
    }

    request->address = 0U;
    request->pdu = NULL;
    request->pdu_length = 0U;

    if (adu_length < MODBUS_ASCII_MIN_ADU_LENGTH)
    {
        return MODBUS_ASCII_STATUS_FRAME_TOO_SHORT;
    }
    if (adu_length > MODBUS_ASCII_MAX_ADU_LENGTH)
    {
        return MODBUS_ASCII_STATUS_FRAME_TOO_LONG;
    }
    if (adu[0] != MODBUS_ASCII_START_CHARACTER)
    {
        return MODBUS_ASCII_STATUS_MISSING_START;
    }
    if ((adu[adu_length - 2U] != MODBUS_ASCII_CR_CHARACTER) ||
        (adu[adu_length - 1U] != MODBUS_ASCII_LF_CHARACTER))
    {
        return MODBUS_ASCII_STATUS_MISSING_CRLF;
    }

    hex_length = adu_length - 3U;
    if ((hex_length & 1U) != 0U)
    {
        return MODBUS_ASCII_STATUS_ODD_HEX_LENGTH;
    }

    decoded_length = hex_length / 2U;
    if (decoded_length > MODBUS_ASCII_MAX_BINARY_LENGTH)
    {
        return MODBUS_ASCII_STATUS_FRAME_TOO_LONG;
    }
    if (decode_capacity < decoded_length)
    {
        return MODBUS_ASCII_STATUS_BUFFER_TOO_SMALL;
    }

    for (index = 0U; index < decoded_length; index++)
    {
        if ((!HexToNibble(adu[1U + (index * 2U)], &high_nibble)) ||
            (!HexToNibble(adu[2U + (index * 2U)], &low_nibble)))
        {
            return MODBUS_ASCII_STATUS_INVALID_HEX;
        }
        decode_buffer[index] = (uint8_t)((high_nibble << 4U) | low_nibble);
    }

    if (!ModbusLrc_IsFrameValid(decode_buffer, decoded_length))
    {
        return MODBUS_ASCII_STATUS_LRC_ERROR;
    }

    request->address = decode_buffer[0];
    request->pdu = &decode_buffer[1];
    request->pdu_length = decoded_length - 2U;
    return MODBUS_ASCII_STATUS_OK;
}

ModbusAsciiStatus_t ModbusAsciiFramer_Encode(
    uint8_t address,
    const uint8_t *pdu,
    size_t pdu_length,
    uint8_t *adu,
    size_t adu_capacity,
    size_t *adu_length)
{
    uint8_t lrc;
    uint8_t sum;
    size_t required_length;
    size_t index;
    size_t output_index;

    if ((pdu == NULL) || (adu == NULL) || (adu_length == NULL) ||
        (pdu_length == 0U))
    {
        return MODBUS_ASCII_STATUS_INVALID_ARGUMENT;
    }

    *adu_length = 0U;
    if (pdu_length > MODBUS_ASCII_MAX_PDU_LENGTH)
    {
        return MODBUS_ASCII_STATUS_FRAME_TOO_LONG;
    }

    required_length = 1U + (2U * (pdu_length + 2U)) + 2U;
    if (adu_capacity < required_length)
    {
        return MODBUS_ASCII_STATUS_BUFFER_TOO_SMALL;
    }

    adu[0] = MODBUS_ASCII_START_CHARACTER;
    EncodeByte(address, &adu[1]);

    sum = address;
    output_index = 3U;
    for (index = 0U; index < pdu_length; index++)
    {
        EncodeByte(pdu[index], &adu[output_index]);
        output_index += 2U;
        sum = (uint8_t)(sum + pdu[index]);
    }

    lrc = (uint8_t)(0U - sum);
    EncodeByte(lrc, &adu[output_index]);
    output_index += 2U;
    adu[output_index++] = MODBUS_ASCII_CR_CHARACTER;
    adu[output_index++] = MODBUS_ASCII_LF_CHARACTER;

    *adu_length = output_index;
    return MODBUS_ASCII_STATUS_OK;
}
