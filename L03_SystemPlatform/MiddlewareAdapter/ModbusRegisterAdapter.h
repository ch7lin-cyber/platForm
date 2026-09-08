#ifndef MODBUS_REGISTER_ADAPTER_H
#define MODBUS_REGISTER_ADAPTER_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MODBUS_SERIAL_REGISTER_BASE          (0x1200U)
#define MODBUS_SERIAL_REGISTER_PORT_STRIDE   (0x0010U)
#define MODBUS_SERIAL_REGISTER_PORT_COUNT    (2U)

#define MODBUS_SERIAL_APPLY_KEY              (0xA5A5U)
#define MODBUS_SERIAL_UNIT_ID_MIN            (1U)
#define MODBUS_SERIAL_UNIT_ID_MAX            (247U)
#define MODBUS_SERIAL_TIMEOUT_MS_MIN         (1U)
#define MODBUS_SERIAL_TIMEOUT_MS_MAX         (60000U)

typedef enum
{
    MODBUS_SERIAL_BAUD_4800 = 0,
    MODBUS_SERIAL_BAUD_9600,
    MODBUS_SERIAL_BAUD_19200,
    MODBUS_SERIAL_BAUD_38400,
    MODBUS_SERIAL_BAUD_57600,
    MODBUS_SERIAL_BAUD_115200
} ModbusSerialBaudCode_t;

typedef enum
{
    MODBUS_SERIAL_REGISTER_BAUD_CODE = 0x00,
    MODBUS_SERIAL_REGISTER_DATA_BITS = 0x01,
    MODBUS_SERIAL_REGISTER_PARITY = 0x02,
    MODBUS_SERIAL_REGISTER_STOP_BITS = 0x03,
    MODBUS_SERIAL_REGISTER_PROTOCOL = 0x04,
    MODBUS_SERIAL_REGISTER_ROLE = 0x05,
    MODBUS_SERIAL_REGISTER_UNIT_ID = 0x06,
    MODBUS_SERIAL_REGISTER_RESPONSE_TIMEOUT_MS = 0x07,
    MODBUS_SERIAL_REGISTER_APPLY = 0x08,
    MODBUS_SERIAL_REGISTER_STATUS = 0x09,
    MODBUS_SERIAL_REGISTER_REVISION = 0x0A
} ModbusSerialRegisterOffset_t;

typedef enum
{
    MODBUS_REGISTER_ACCESS_READ_ONLY = 0,
    MODBUS_REGISTER_ACCESS_WRITE_ONLY,
    MODBUS_REGISTER_ACCESS_READ_WRITE
} ModbusRegisterAccess_t;

typedef enum
{
    MODBUS_SERIAL_STATUS_ACTIVE = 0,
    MODBUS_SERIAL_STATUS_PENDING,
    MODBUS_SERIAL_STATUS_WAITING_TX_COMPLETE,
    MODBUS_SERIAL_STATUS_APPLYING,
    MODBUS_SERIAL_STATUS_ERROR
} ModbusSerialConfigurationStatus_t;

typedef struct
{
    uint16_t address;
    uint8_t port;
    ModbusSerialRegisterOffset_t field;
    ModbusRegisterAccess_t access;
} ModbusSerialRegisterInfo_t;

bool ModbusRegisterAdapter_GetSerialAddress(
    uint8_t port,
    ModbusSerialRegisterOffset_t field,
    uint16_t *address);
bool ModbusRegisterAdapter_ResolveSerialAddress(
    uint16_t address,
    ModbusSerialRegisterInfo_t *information);
bool ModbusRegisterAdapter_IsSerialValueValid(
    ModbusSerialRegisterOffset_t field,
    uint16_t value);
bool ModbusRegisterAdapter_BaudCodeToRate(
    uint16_t baud_code,
    uint32_t *baud_rate);
bool ModbusRegisterAdapter_BaudRateToCode(
    uint32_t baud_rate,
    uint16_t *baud_code);

#ifdef __cplusplus
}
#endif

#endif
