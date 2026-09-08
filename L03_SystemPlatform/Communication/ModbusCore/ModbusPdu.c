#include "ModbusPdu.h"
#include <stddef.h>
uint16_t ModbusPdu_ReadU16(const uint8_t *data) { return (uint16_t)(((uint16_t)data[0]<<8U)|data[1]); }
void ModbusPdu_WriteU16(uint8_t *data,uint16_t value) { data[0]=(uint8_t)(value>>8U); data[1]=(uint8_t)value; }
ModbusPduStatus_t ModbusPdu_BuildException(uint8_t function_code,
    ModbusExceptionCode_t exception,uint8_t *response_pdu,size_t response_capacity,
    size_t *response_length)
{
    if ((response_pdu==NULL)||(response_length==NULL)||(exception==MODBUS_EXCEPTION_NONE)) return MODBUS_PDU_STATUS_INVALID_ARGUMENT;
    *response_length=0U;
    if (response_capacity<2U) return MODBUS_PDU_STATUS_RESPONSE_TOO_SMALL;
    response_pdu[0]=(uint8_t)(function_code|0x80U); response_pdu[1]=(uint8_t)exception; *response_length=2U;
    return MODBUS_PDU_STATUS_OK;
}
