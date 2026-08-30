#include "SerialConfiguration.h"

#include <stddef.h>

#define SERIAL_DEFAULT_BAUD_RATE          (115200UL)
#define SERIAL_DEFAULT_RESPONSE_TIMEOUT   (1000UL)

static bool SerialConfiguration_IsProtocolValid(SerialProtocol_t protocol)
{
    return ((protocol == SERIAL_PROTOCOL_RAW) ||
            (protocol == SERIAL_PROTOCOL_MODBUS_RTU) ||
            (protocol == SERIAL_PROTOCOL_MODBUS_ASCII));
}

static bool SerialConfiguration_IsRoleValid(SerialRole_t role)
{
    return ((role == SERIAL_ROLE_GENERIC) ||
            (role == SERIAL_ROLE_MODBUS_SLAVE) ||
            (role == SERIAL_ROLE_MODBUS_MASTER));
}

void SerialConfiguration_SetRtuDefault(SerialConfiguration_t *configuration,
                                       SerialRole_t role)
{
    if (configuration != NULL)
    {
        configuration->line.baud_rate = SERIAL_DEFAULT_BAUD_RATE;
        configuration->line.data_bits = HAL_SERIAL_DATA_BITS_8;
        configuration->line.parity = HAL_SERIAL_PARITY_EVEN;
        configuration->line.stop_bits = HAL_SERIAL_STOP_BITS_1;
        configuration->protocol = SERIAL_PROTOCOL_MODBUS_RTU;
        configuration->role = role;
        configuration->response_timeout_ms = SERIAL_DEFAULT_RESPONSE_TIMEOUT;
    }
}

void SerialConfiguration_SetAsciiDefault(SerialConfiguration_t *configuration,
                                         SerialRole_t role)
{
    if (configuration != NULL)
    {
        configuration->line.baud_rate = SERIAL_DEFAULT_BAUD_RATE;
        configuration->line.data_bits = HAL_SERIAL_DATA_BITS_7;
        configuration->line.parity = HAL_SERIAL_PARITY_EVEN;
        configuration->line.stop_bits = HAL_SERIAL_STOP_BITS_1;
        configuration->protocol = SERIAL_PROTOCOL_MODBUS_ASCII;
        configuration->role = role;
        configuration->response_timeout_ms = SERIAL_DEFAULT_RESPONSE_TIMEOUT;
    }
}

bool SerialConfiguration_IsValid(const SerialConfiguration_t *configuration)
{
    if ((configuration == NULL) ||
        (!HalSerial_IsConfigValid(&configuration->line)) ||
        (!SerialConfiguration_IsProtocolValid(configuration->protocol)) ||
        (!SerialConfiguration_IsRoleValid(configuration->role)))
    {
        return false;
    }

    if ((configuration->protocol != SERIAL_PROTOCOL_RAW) &&
        (configuration->role == SERIAL_ROLE_GENERIC))
    {
        return false;
    }

    if ((configuration->protocol == SERIAL_PROTOCOL_MODBUS_RTU) &&
        (configuration->line.data_bits != HAL_SERIAL_DATA_BITS_8))
    {
        return false;
    }

    return (configuration->response_timeout_ms > 0U);
}

uint32_t SerialConfiguration_GetCharacterTimeUs(
    const SerialConfiguration_t *configuration)
{
    uint32_t bits;

    if (!SerialConfiguration_IsValid(configuration))
    {
        return 0U;
    }
    bits = 1U + (uint32_t)configuration->line.data_bits +
           (uint32_t)configuration->line.stop_bits;
    if (configuration->line.parity != HAL_SERIAL_PARITY_NONE)
    {
        bits++;
    }
    return ((bits * 1000000UL) + configuration->line.baud_rate - 1UL) /
           configuration->line.baud_rate;
}

uint32_t SerialConfiguration_GetRtuFrameGapUs(
    const SerialConfiguration_t *configuration)
{
    uint32_t character_time_us;

    if ((configuration == NULL) ||
        (configuration->protocol != SERIAL_PROTOCOL_MODBUS_RTU))
    {
        return 0U;
    }

    /* Modbus Serial Line specification: fixed 1.75 ms above 19200 baud. */
    if (configuration->line.baud_rate > 19200UL)
    {
        return 1750UL;
    }
    character_time_us = SerialConfiguration_GetCharacterTimeUs(configuration);
    return ((character_time_us * 35UL) + 9UL) / 10UL;
}
