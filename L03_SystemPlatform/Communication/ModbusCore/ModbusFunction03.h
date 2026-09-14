#ifndef MODBUS_FUNCTION_03_H
#define MODBUS_FUNCTION_03_H

#include <stddef.h>
#include <stdint.h>

#include "ModbusPdu.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MODBUS_FUNCTION_READ_HOLDING_REGISTERS (0x03U)
#define MODBUS_FUNCTION_READ_MAX_REGISTERS     (125U)
#define MODBUS_FUNCTION_READ_REQUEST_LENGTH    (5U)

typedef ModbusExceptionCode_t (*ModbusReadRegistersCallback_t)(
    void *context,
    uint16_t starting_address,
    uint16_t quantity,
    uint16_t *values);

ModbusPduStatus_t ModbusFunction03_Process(
    const uint8_t *request_pdu,
    size_t request_length,
    ModbusReadRegistersCallback_t read_registers,
    void *context,
    uint8_t *response_pdu,
    size_t response_capacity,
    size_t *response_length);

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_FUNCTION_03_H */
