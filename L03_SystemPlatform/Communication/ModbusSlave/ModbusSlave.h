#ifndef MODBUS_SLAVE_H
#define MODBUS_SLAVE_H

#include <stddef.h>
#include <stdint.h>

#include "ModbusFunction03.h"
#include "ModbusFunction06.h"
#include "ModbusFunction10.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MODBUS_SLAVE_ADDRESS_MIN (1U)
#define MODBUS_SLAVE_ADDRESS_MAX (247U)
#define MODBUS_BROADCAST_ADDRESS (0U)

typedef enum
{
    MODBUS_SLAVE_RESULT_RESPONSE_READY = 0,
    MODBUS_SLAVE_RESULT_PROCESSED_NO_RESPONSE,
    MODBUS_SLAVE_RESULT_IGNORED,
    MODBUS_SLAVE_RESULT_CRC_ERROR,
    MODBUS_SLAVE_RESULT_LRC_ERROR,
    MODBUS_SLAVE_RESULT_INVALID_ARGUMENT,
    MODBUS_SLAVE_RESULT_RESPONSE_TOO_SMALL
} ModbusSlaveResult_t;

typedef struct
{
    ModbusReadRegistersCallback_t read_holding_registers;
    ModbusReadRegistersCallback_t read_input_registers;
    ModbusFunction06WriteCallback_t write_single_register;
    ModbusFunction10WriteCallback_t write_multiple_registers;
    void *context;
} ModbusSlaveRegisterInterface_t;

typedef struct
{
    uint8_t unit_address;
    ModbusSlaveRegisterInterface_t registers;
} ModbusSlaveConfig_t;

ModbusSlaveResult_t ModbusSlave_ProcessRtuRequest(
    const ModbusSlaveConfig_t *config,
    const uint8_t *request_adu,
    size_t request_length,
    uint8_t *response_adu,
    size_t response_capacity,
    size_t *response_length);

ModbusSlaveResult_t ModbusSlave_ProcessAsciiRequest(
    const ModbusSlaveConfig_t *config,
    const uint8_t *request_adu,
    size_t request_length,
    uint8_t *response_adu,
    size_t response_capacity,
    size_t *response_length);

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_SLAVE_H */
