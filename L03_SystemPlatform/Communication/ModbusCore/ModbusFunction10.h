#ifndef MODBUS_FUNCTION_10_H
#define MODBUS_FUNCTION_10_H
#include <stddef.h>
#include <stdint.h>
#include "ModbusPdu.h"
#define MODBUS_FUNCTION_WRITE_MULTIPLE_REGISTERS (0x10U)
#define MODBUS_FUNCTION_10_MAX_REGISTERS (123U)
/* Callback must validate the complete request before committing any value. */
typedef ModbusExceptionCode_t (*ModbusFunction10WriteCallback_t)(void *context,
    uint16_t starting_address,const uint16_t *values,uint16_t quantity);
ModbusPduStatus_t ModbusFunction10_Process(const uint8_t *request_pdu,size_t request_length,
    ModbusFunction10WriteCallback_t write_registers,void *context,uint8_t *response_pdu,
    size_t response_capacity,size_t *response_length);
#endif
