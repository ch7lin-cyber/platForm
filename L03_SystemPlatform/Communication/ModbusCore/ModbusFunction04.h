#ifndef MODBUS_FUNCTION_04_H
#define MODBUS_FUNCTION_04_H

#include <stddef.h>
#include <stdint.h>

#include "ModbusFunction03.h"
#include "ModbusPdu.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MODBUS_FUNCTION_READ_INPUT_REGISTERS (0x04U)

ModbusPduStatus_t ModbusFunction04_Process(
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

#endif /* MODBUS_FUNCTION_04_H */
