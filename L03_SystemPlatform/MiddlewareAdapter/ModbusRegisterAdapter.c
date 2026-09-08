#include "ModbusRegisterAdapter.h"

#include <stddef.h>

#include "EventService.h"

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

typedef struct
{
    ModbusSerialPortConfiguration_t active;
    ModbusSerialPortConfiguration_t pending;
    ModbusSerialConfigurationStatus_t status;
    uint16_t revision;
    bool initialized;
    bool apply_requested;
} SerialConfigurationInstance_t;

static SerialConfigurationInstance_t
    g_serial_configuration[MODBUS_SERIAL_REGISTER_PORT_COUNT];

static bool IsPortConfigurationValid(
    const ModbusSerialPortConfiguration_t *configuration)
{
    return ((configuration != NULL) &&
            SerialConfiguration_IsValid(&configuration->serial) &&
            (configuration->unit_id >= MODBUS_SERIAL_UNIT_ID_MIN) &&
            (configuration->unit_id <= MODBUS_SERIAL_UNIT_ID_MAX));
}

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

static bool SetPendingField(
    ModbusSerialPortConfiguration_t *configuration,
    ModbusSerialRegisterOffset_t field,
    uint16_t value)
{
    uint32_t baud_rate;

    if ((configuration == NULL) ||
        (!ModbusRegisterAdapter_IsSerialValueValid(field, value)))
    {
        return false;
    }

    switch (field)
    {
        case MODBUS_SERIAL_REGISTER_BAUD_CODE:
            if (!ModbusRegisterAdapter_BaudCodeToRate(value, &baud_rate))
            {
                return false;
            }
            configuration->serial.line.baud_rate = baud_rate;
            break;
        case MODBUS_SERIAL_REGISTER_DATA_BITS:
            configuration->serial.line.data_bits = (HalSerialDataBits_t)value;
            break;
        case MODBUS_SERIAL_REGISTER_PARITY:
            configuration->serial.line.parity = (HalSerialParity_t)value;
            break;
        case MODBUS_SERIAL_REGISTER_STOP_BITS:
            configuration->serial.line.stop_bits = (HalSerialStopBits_t)value;
            break;
        case MODBUS_SERIAL_REGISTER_PROTOCOL:
            configuration->serial.protocol = (SerialProtocol_t)value;
            break;
        case MODBUS_SERIAL_REGISTER_ROLE:
            configuration->serial.role = (SerialRole_t)value;
            break;
        case MODBUS_SERIAL_REGISTER_UNIT_ID:
            configuration->unit_id = value;
            break;
        case MODBUS_SERIAL_REGISTER_RESPONSE_TIMEOUT_MS:
            configuration->serial.response_timeout_ms = value;
            break;
        default:
            return false;
    }
    return true;
}

static bool GetPendingField(
    const SerialConfigurationInstance_t *instance,
    ModbusSerialRegisterOffset_t field,
    uint16_t *value)
{
    if ((instance == NULL) || (value == NULL))
    {
        return false;
    }

    switch (field)
    {
        case MODBUS_SERIAL_REGISTER_BAUD_CODE:
            return ModbusRegisterAdapter_BaudRateToCode(
                instance->pending.serial.line.baud_rate, value);
        case MODBUS_SERIAL_REGISTER_DATA_BITS:
            *value = (uint16_t)instance->pending.serial.line.data_bits;
            break;
        case MODBUS_SERIAL_REGISTER_PARITY:
            *value = (uint16_t)instance->pending.serial.line.parity;
            break;
        case MODBUS_SERIAL_REGISTER_STOP_BITS:
            *value = (uint16_t)instance->pending.serial.line.stop_bits;
            break;
        case MODBUS_SERIAL_REGISTER_PROTOCOL:
            *value = (uint16_t)instance->pending.serial.protocol;
            break;
        case MODBUS_SERIAL_REGISTER_ROLE:
            *value = (uint16_t)instance->pending.serial.role;
            break;
        case MODBUS_SERIAL_REGISTER_UNIT_ID:
            *value = instance->pending.unit_id;
            break;
        case MODBUS_SERIAL_REGISTER_RESPONSE_TIMEOUT_MS:
            *value = (uint16_t)instance->pending.serial.response_timeout_ms;
            break;
        case MODBUS_SERIAL_REGISTER_STATUS:
            *value = (uint16_t)instance->status;
            break;
        case MODBUS_SERIAL_REGISTER_REVISION:
            *value = instance->revision;
            break;
        default:
            return false;
    }
    return true;
}

bool ModbusRegisterAdapter_InitializeSerialPort(
    uint8_t port,
    const ModbusSerialPortConfiguration_t *configuration)
{
    SerialConfigurationInstance_t *instance;

    if ((port >= MODBUS_SERIAL_REGISTER_PORT_COUNT) ||
        (!IsPortConfigurationValid(configuration)))
    {
        return false;
    }

    instance = &g_serial_configuration[port];
    instance->active = *configuration;
    instance->pending = *configuration;
    instance->status = MODBUS_SERIAL_STATUS_ACTIVE;
    instance->revision = 0U;
    instance->apply_requested = false;
    instance->initialized = true;
    return true;
}

bool ModbusRegisterAdapter_GetActiveSerialConfiguration(
    uint8_t port,
    ModbusSerialPortConfiguration_t *configuration)
{
    if ((port >= MODBUS_SERIAL_REGISTER_PORT_COUNT) ||
        (configuration == NULL) ||
        (!g_serial_configuration[port].initialized))
    {
        return false;
    }
    *configuration = g_serial_configuration[port].active;
    return true;
}

bool ModbusRegisterAdapter_GetPendingSerialConfiguration(
    uint8_t port,
    ModbusSerialPortConfiguration_t *configuration)
{
    if ((port >= MODBUS_SERIAL_REGISTER_PORT_COUNT) ||
        (configuration == NULL) ||
        (!g_serial_configuration[port].initialized))
    {
        return false;
    }
    *configuration = g_serial_configuration[port].pending;
    return true;
}

ModbusExceptionCode_t ModbusRegisterAdapter_ReadSerialRegister(
    uint16_t address,
    uint16_t *value)
{
    ModbusSerialRegisterInfo_t information;
    const SerialConfigurationInstance_t *instance;

    if ((value == NULL) ||
        (!ModbusRegisterAdapter_ResolveSerialAddress(address, &information)) ||
        (information.access == MODBUS_REGISTER_ACCESS_WRITE_ONLY))
    {
        return MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }
    instance = &g_serial_configuration[information.port];
    if (!instance->initialized)
    {
        return MODBUS_EXCEPTION_SERVER_DEVICE_FAILURE;
    }
    if (!GetPendingField(instance, information.field, value))
    {
        return MODBUS_EXCEPTION_SERVER_DEVICE_FAILURE;
    }
    return MODBUS_EXCEPTION_NONE;
}

ModbusExceptionCode_t ModbusRegisterAdapter_WriteMultipleRegisters(
    void *context,
    uint16_t starting_address,
    const uint16_t *values,
    uint16_t quantity)
{
    ModbusSerialRegisterInfo_t information;
    SerialConfigurationInstance_t *instance;
    ModbusSerialPortConfiguration_t candidate;
    uint16_t index;
    uint8_t port;
    bool apply_requested = false;

    (void)context;
    if ((values == NULL) || (quantity == 0U) ||
        (!ModbusRegisterAdapter_ResolveSerialAddress(starting_address,
                                                     &information)))
    {
        return MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }

    port = information.port;
    instance = &g_serial_configuration[port];
    if (!instance->initialized)
    {
        return MODBUS_EXCEPTION_SERVER_DEVICE_FAILURE;
    }
    if ((instance->status == MODBUS_SERIAL_STATUS_WAITING_TX_COMPLETE) ||
        (instance->status == MODBUS_SERIAL_STATUS_APPLYING))
    {
        return MODBUS_EXCEPTION_SERVER_DEVICE_BUSY;
    }

    candidate = instance->pending;
    for (index = 0U; index < quantity; index++)
    {
        uint16_t address = (uint16_t)(starting_address + index);

        if ((address < starting_address) ||
            (!ModbusRegisterAdapter_ResolveSerialAddress(address,
                                                         &information)) ||
            (information.port != port) ||
            (information.access == MODBUS_REGISTER_ACCESS_READ_ONLY))
        {
            return MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
        }

        if (information.field == MODBUS_SERIAL_REGISTER_APPLY)
        {
            if (values[index] != MODBUS_SERIAL_APPLY_KEY)
            {
                return MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;
            }
            apply_requested = true;
        }
        else if (!SetPendingField(&candidate, information.field,
                                  values[index]))
        {
            return MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;
        }
    }

    if (apply_requested && (!IsPortConfigurationValid(&candidate)))
    {
        return MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;
    }

    instance->pending = candidate;
    if (apply_requested)
    {
        instance->apply_requested = true;
        instance->status = MODBUS_SERIAL_STATUS_WAITING_TX_COMPLETE;
    }
    else
    {
        instance->status = MODBUS_SERIAL_STATUS_PENDING;
    }
    return MODBUS_EXCEPTION_NONE;
}

ModbusExceptionCode_t ModbusRegisterAdapter_WriteSingleRegister(
    void *context,
    uint16_t address,
    uint16_t value)
{
    return ModbusRegisterAdapter_WriteMultipleRegisters(
        context, address, &value, 1U);
}

bool ModbusRegisterAdapter_IsApplyRequested(uint8_t port)
{
    return ((port < MODBUS_SERIAL_REGISTER_PORT_COUNT) &&
            g_serial_configuration[port].initialized &&
            g_serial_configuration[port].apply_requested);
}

bool ModbusRegisterAdapter_BeginApply(
    uint8_t port,
    ModbusSerialPortConfiguration_t *configuration)
{
    SerialConfigurationInstance_t *instance;

    if ((port >= MODBUS_SERIAL_REGISTER_PORT_COUNT) ||
        (configuration == NULL))
    {
        return false;
    }
    instance = &g_serial_configuration[port];
    if ((!instance->initialized) || (!instance->apply_requested) ||
        (instance->status != MODBUS_SERIAL_STATUS_WAITING_TX_COMPLETE))
    {
        return false;
    }

    *configuration = instance->pending;
    instance->status = MODBUS_SERIAL_STATUS_APPLYING;
    return true;
}

bool ModbusRegisterAdapter_CompleteApply(uint8_t port, bool successful)
{
    SerialConfigurationInstance_t *instance;
    EventSerialConfiguration_t old_configuration;
    EventSerialConfiguration_t new_configuration;
    uint32_t event_id;

    if (port >= MODBUS_SERIAL_REGISTER_PORT_COUNT)
    {
        return false;
    }
    instance = &g_serial_configuration[port];
    if ((!instance->initialized) ||
        (instance->status != MODBUS_SERIAL_STATUS_APPLYING))
    {
        return false;
    }

    instance->apply_requested = false;
    if (successful)
    {
        old_configuration.serial = instance->active.serial;
        old_configuration.unit_id = instance->active.unit_id;
        instance->active = instance->pending;
        instance->revision++;
        instance->status = MODBUS_SERIAL_STATUS_ACTIVE;

        new_configuration.serial = instance->active.serial;
        new_configuration.unit_id = instance->active.unit_id;
        if (EventService_RaiseSerialConfigurationChanged(
                port, instance->revision,
                &old_configuration, &new_configuration, &event_id))
        {
            /* Successful hardware completion is Communication's ACK. */
            (void)EventService_Acknowledge(event_id,
                                           EVENT_ACK_COMMUNICATION);
        }
    }
    else
    {
        instance->status = MODBUS_SERIAL_STATUS_ERROR;
    }
    return true;
}

bool ModbusRegisterAdapter_CancelApply(uint8_t port)
{
    SerialConfigurationInstance_t *instance;

    if (port >= MODBUS_SERIAL_REGISTER_PORT_COUNT)
    {
        return false;
    }
    instance = &g_serial_configuration[port];
    if ((!instance->initialized) ||
        (instance->status != MODBUS_SERIAL_STATUS_WAITING_TX_COMPLETE))
    {
        return false;
    }

    instance->apply_requested = false;
    instance->status = MODBUS_SERIAL_STATUS_ERROR;
    return true;
}

bool ModbusRegisterAdapter_DiscardPending(uint8_t port)
{
    SerialConfigurationInstance_t *instance;

    if (port >= MODBUS_SERIAL_REGISTER_PORT_COUNT)
    {
        return false;
    }
    instance = &g_serial_configuration[port];
    if ((!instance->initialized) ||
        (instance->status == MODBUS_SERIAL_STATUS_WAITING_TX_COMPLETE) ||
        (instance->status == MODBUS_SERIAL_STATUS_APPLYING))
    {
        return false;
    }

    instance->pending = instance->active;
    instance->apply_requested = false;
    instance->status = MODBUS_SERIAL_STATUS_ACTIVE;
    return true;
}
