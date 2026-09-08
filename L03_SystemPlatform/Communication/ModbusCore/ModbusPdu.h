#ifndef MODBUS_PDU_H
#define MODBUS_PDU_H
#include <stddef.h>
#include <stdint.h>
typedef enum { MODBUS_EXCEPTION_NONE=0x00, MODBUS_EXCEPTION_ILLEGAL_FUNCTION=0x01,
    MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS=0x02, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE=0x03,
    MODBUS_EXCEPTION_SERVER_DEVICE_FAILURE=0x04, MODBUS_EXCEPTION_SERVER_DEVICE_BUSY=0x06
} ModbusExceptionCode_t;
typedef enum { MODBUS_PDU_STATUS_OK=0, MODBUS_PDU_STATUS_INVALID_ARGUMENT,
    MODBUS_PDU_STATUS_RESPONSE_TOO_SMALL } ModbusPduStatus_t;
uint16_t ModbusPdu_ReadU16(const uint8_t *data);
void ModbusPdu_WriteU16(uint8_t *data, uint16_t value);
ModbusPduStatus_t ModbusPdu_BuildException(uint8_t function_code,
    ModbusExceptionCode_t exception, uint8_t *response_pdu,
    size_t response_capacity, size_t *response_length);
#endif
