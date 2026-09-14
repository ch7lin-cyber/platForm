#include "ModbusLrc.h"

#include <stddef.h>

uint8_t ModbusLrc_Calculate(const uint8_t *data, size_t length)
{
    uint8_t sum = 0U;
    size_t index;

    if ((data == NULL) && (length != 0U))
    {
        return 0U;
    }

    for (index = 0U; index < length; index++)
    {
        sum = (uint8_t)(sum + data[index]);
    }

    return (uint8_t)(0U - sum);
}

bool ModbusLrc_IsFrameValid(const uint8_t *data_with_lrc, size_t length)
{
    uint8_t sum = 0U;
    size_t index;

    if ((data_with_lrc == NULL) || (length < 3U))
    {
        return false;
    }

    for (index = 0U; index < length; index++)
    {
        sum = (uint8_t)(sum + data_with_lrc[index]);
    }

    return (sum == 0U);
}

bool ModbusLrc_Append(uint8_t *frame, size_t payload_length, size_t capacity)
{
    if ((frame == NULL) || (capacity < 1U) ||
        (payload_length > (capacity - 1U)))
    {
        return false;
    }

    frame[payload_length] = ModbusLrc_Calculate(frame, payload_length);
    return true;
}
