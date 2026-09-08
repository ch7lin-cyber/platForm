#include "ModbusRegisterAdapter.h"

#include <stddef.h>

typedef struct
{
    ModbusSerialRegisterOffset_t field;
    ModbusRegisterAccess_t access;
} SerialRegisterDefinition_t;

static const SerialRegisterDefinition_t g_serial_register_definitions[] =
{
    {MODBUS_SERIAL_REGISTER_BAUD_CODE, MODBUS_REGISTER_ACCESS_READ_WRITE},
    {MODBUS_SERIAL_REGISTER_DATA_BITS, MODBUS_REGISTER_ACCESS_READ_WRITE},
    {MODBUS_SERIAL_REGISTER_PARITY, MODBUS_REGISTER_ACCESS_READ_WRITE},
    {MODBUS_SERIAL_REGISTER_STOP_BITS, MODBUS_REGISTER_ACCESS_READ_WRITE},
    {MODBUS_SERIAL_REGISTER_PROTOCOL, MODBUS_REGISTER_ACCESS_READ_WRITE},
    {MODBUS_SERIAL_REGISTER_ROLE, MODBUS_REGISTER_ACCESS_READ_WRITE},
    {MODBUS_SERIAL_REGISTER_UNIT_ID, MODBUS_REGISTER_ACCESS_READ_WRITE},
    {MODBUS_SERIAL_REGISTER_RESPONSE_TIMEOUT_MS,
     MODBUS_REGISTER_ACCESS_READ_WRITE},
    {MODBUS_SERIAL_REGISTER_APPLY, MODBUS_REGISTER_ACCESS_WRITE_ONLY},
    {MODBUS_SERIAL_REGISTER_STATUS, MODBUS_REGISTER_ACCESS_READ_ONLY},
    {MODBUS_SERIAL_REGISTER_REVISION, MODBUS_REGISTER_ACCESS_READ_ONLY}
};

static const uint32_t g_serial_baud_rates[] =
{
    4800UL,
    9600UL,
    19200UL,
    38400UL,
    57600UL,
    115200UL
};

static const SerialRegisterDefinition_t *FindDefinition(uint16_t offset)
{
    size_t index;

    for (index = 0U;
         index < (sizeof(g_serial_register_definitions) /
                  sizeof(g_serial_register_definitions[0]));
         index++)
    {
        if ((uint16_t)g_serial_register_definitions[index].field == offset)
        {
            return &g_serial_register_definitions[index];
        }
    }
    return NULL;
}

bool ModbusRegisterAdapter_GetSerialAddress(
    uint8_t port,
    ModbusSerialRegisterOffset_t field,
    uint16_t *address)
{
    if ((port >= MODBUS_SERIAL_REGISTER_PORT_COUNT) ||
        (address == NULL) ||
        (FindDefinition((uint16_t)field) == NULL))
    {
        return false;
    }

    *address = (uint16_t)(MODBUS_SERIAL_REGISTER_BASE +
                          ((uint16_t)port * MODBUS_SERIAL_REGISTER_PORT_STRIDE) +
                          (uint16_t)field);
    return true;
}

bool ModbusRegisterAdapter_ResolveSerialAddress(
    uint16_t address,
    ModbusSerialRegisterInfo_t *information)
{
    const SerialRegisterDefinition_t *definition;
    uint16_t relative_address;
    uint16_t port;
    uint16_t offset;

    if ((information == NULL) || (address < MODBUS_SERIAL_REGISTER_BASE))
    {
        return false;
    }

    relative_address = (uint16_t)(address - MODBUS_SERIAL_REGISTER_BASE);
    port = (uint16_t)(relative_address / MODBUS_SERIAL_REGISTER_PORT_STRIDE);
    offset = (uint16_t)(relative_address % MODBUS_SERIAL_REGISTER_PORT_STRIDE);
    definition = FindDefinition(offset);

    if ((port >= MODBUS_SERIAL_REGISTER_PORT_COUNT) || (definition == NULL))
    {
        return false;
    }

    information->address = address;
    information->port = (uint8_t)port;
    information->field = definition->field;
    information->access = definition->access;
    return true;
}

bool ModbusRegisterAdapter_IsSerialValueValid(
    ModbusSerialRegisterOffset_t field,
    uint16_t value)
{
    switch (field)
    {
        case MODBUS_SERIAL_REGISTER_BAUD_CODE:
            return (value <= (uint16_t)MODBUS_SERIAL_BAUD_115200);
        case MODBUS_SERIAL_REGISTER_DATA_BITS:
            return ((value == 7U) || (value == 8U));
        case MODBUS_SERIAL_REGISTER_PARITY:
            return (value <= 2U);
        case MODBUS_SERIAL_REGISTER_STOP_BITS:
            return ((value == 1U) || (value == 2U));
        case MODBUS_SERIAL_REGISTER_PROTOCOL:
            return (value <= 2U);
        case MODBUS_SERIAL_REGISTER_ROLE:
            return (value <= 2U);
        case MODBUS_SERIAL_REGISTER_UNIT_ID:
            return ((value >= MODBUS_SERIAL_UNIT_ID_MIN) &&
                    (value <= MODBUS_SERIAL_UNIT_ID_MAX));
        case MODBUS_SERIAL_REGISTER_RESPONSE_TIMEOUT_MS:
            return ((value >= MODBUS_SERIAL_TIMEOUT_MS_MIN) &&
                    (value <= MODBUS_SERIAL_TIMEOUT_MS_MAX));
        case MODBUS_SERIAL_REGISTER_APPLY:
            return (value == MODBUS_SERIAL_APPLY_KEY);
        case MODBUS_SERIAL_REGISTER_STATUS:
        case MODBUS_SERIAL_REGISTER_REVISION:
        default:
            return false;
    }
}

bool ModbusRegisterAdapter_BaudCodeToRate(
    uint16_t baud_code,
    uint32_t *baud_rate)
{
    if ((baud_rate == NULL) ||
        (baud_code >= (sizeof(g_serial_baud_rates) /
                      sizeof(g_serial_baud_rates[0]))))
    {
        return false;
    }
    *baud_rate = g_serial_baud_rates[baud_code];
    return true;
}

bool ModbusRegisterAdapter_BaudRateToCode(
    uint32_t baud_rate,
    uint16_t *baud_code)
{
    size_t index;

    if (baud_code == NULL)
    {
        return false;
    }
    for (index = 0U;
         index < (sizeof(g_serial_baud_rates) /
                  sizeof(g_serial_baud_rates[0]));
         index++)
    {
        if (g_serial_baud_rates[index] == baud_rate)
        {
            *baud_code = (uint16_t)index;
            return true;
        }
    }
    return false;
}
