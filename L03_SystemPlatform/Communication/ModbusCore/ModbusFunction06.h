#ifndef MODBUS_FUNCTION_06_H
#define MODBUS_FUNCTION_06_H
#include <stddef.h>
#include <stdint.h>
#include "ModbusPdu.h"
#define MODBUS_FUNCTION_WRITE_SINGLE_REGISTER (0x06U)
#define MODBUS_FUNCTION_06_REQUEST_LENGTH (5U)
typedef ModbusExceptionCode_t (*ModbusFunction06WriteCallback_t)(void *context,uint16_t address,uint16_t value);
ModbusPduStatus_t ModbusFunction06_Process(const uint8_t *request_pdu,size_t request_length,
    ModbusFunction06WriteCallback_t write_register,void *context,uint8_t *response_pdu,
    size_t response_capacity,size_t *response_length);
#endif
