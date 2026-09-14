#include "ModbusCrc16.h"

#include <stddef.h>

uint16_t ModbusCrc16_Calculate(const uint8_t *data, size_t length)
{
    uint16_t crc = 0xFFFFU;
    size_t index;
    uint8_t bit;

    if ((data == NULL) && (length != 0U))
    {
        return 0U;
    }

    for (index = 0U; index < length; index++)
    {
        crc ^= (uint16_t)data[index];
        for (bit = 0U; bit < 8U; bit++)
        {
            if ((crc & 0x0001U) != 0U)
            {
                crc = (uint16_t)((crc >> 1U) ^ 0xA001U);
            }
            else
            {
                crc >>= 1U;
            }
        }
    }

    return crc;
}

bool ModbusCrc16_IsFrameValid(const uint8_t *frame, size_t length)
{
    uint16_t expected;
    uint16_t received;

    if ((frame == NULL) || (length < 4U))
    {
        return false;
    }

    expected = ModbusCrc16_Calculate(frame, length - 2U);
    received = (uint16_t)frame[length - 2U] |
               ((uint16_t)frame[length - 1U] << 8U);

    return (expected == received);
}

bool ModbusCrc16_Append(uint8_t *frame, size_t payload_length, size_t capacity)
{
    uint16_t crc;

    if ((frame == NULL) || (capacity < 2U) ||
        (payload_length > (capacity - 2U)))
    {
        return false;
    }

    crc = ModbusCrc16_Calculate(frame, payload_length);
    frame[payload_length] = (uint8_t)(crc & 0x00FFU);
    frame[payload_length + 1U] = (uint8_t)(crc >> 8U);

    return true;
}
