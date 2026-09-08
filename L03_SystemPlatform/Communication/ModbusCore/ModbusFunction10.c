#include "ModbusFunction10.h"
#include <stdint.h>
#define MODBUS_FUNCTION_10_HEADER_LENGTH (6U)
#define MODBUS_FUNCTION_10_RESPONSE_LENGTH (5U)
static ModbusExceptionCode_t NormalizeException(ModbusExceptionCode_t e)
{
    switch(e) { case MODBUS_EXCEPTION_NONE: case MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS:
    case MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE: case MODBUS_EXCEPTION_SERVER_DEVICE_FAILURE:
    case MODBUS_EXCEPTION_SERVER_DEVICE_BUSY: return e; default: return MODBUS_EXCEPTION_SERVER_DEVICE_FAILURE; }
}
ModbusPduStatus_t ModbusFunction10_Process(const uint8_t *request_pdu,size_t request_length,
    ModbusFunction10WriteCallback_t write_registers,void *context,uint8_t *response_pdu,
    size_t response_capacity,size_t *response_length)
{
    uint16_t values[MODBUS_FUNCTION_10_MAX_REGISTERS]; ModbusExceptionCode_t e;
    uint16_t address,quantity,index; uint8_t byte_count;
    if ((request_pdu==NULL)||(write_registers==NULL)||(response_pdu==NULL)||(response_length==NULL)) return MODBUS_PDU_STATUS_INVALID_ARGUMENT;
    *response_length=0U;
    if ((request_length<MODBUS_FUNCTION_10_HEADER_LENGTH)||(request_pdu[0]!=MODBUS_FUNCTION_WRITE_MULTIPLE_REGISTERS))
        return ModbusPdu_BuildException(MODBUS_FUNCTION_WRITE_MULTIPLE_REGISTERS,MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE,response_pdu,response_capacity,response_length);
    address=ModbusPdu_ReadU16(&request_pdu[1]); quantity=ModbusPdu_ReadU16(&request_pdu[3]); byte_count=request_pdu[5];
    if ((quantity==0U)||(quantity>MODBUS_FUNCTION_10_MAX_REGISTERS)||
        ((uint16_t)byte_count!=(uint16_t)(quantity*2U))||
        (request_length!=(MODBUS_FUNCTION_10_HEADER_LENGTH+(size_t)byte_count)))
        return ModbusPdu_BuildException(MODBUS_FUNCTION_WRITE_MULTIPLE_REGISTERS,MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE,response_pdu,response_capacity,response_length);
    if (((uint32_t)address+(uint32_t)quantity-1UL)>UINT16_MAX)
        return ModbusPdu_BuildException(MODBUS_FUNCTION_WRITE_MULTIPLE_REGISTERS,MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS,response_pdu,response_capacity,response_length);
    if (response_capacity<MODBUS_FUNCTION_10_RESPONSE_LENGTH) return MODBUS_PDU_STATUS_RESPONSE_TOO_SMALL;
    for (index=0U;index<quantity;index++) values[index]=ModbusPdu_ReadU16(&request_pdu[MODBUS_FUNCTION_10_HEADER_LENGTH+((size_t)index*2U)]);
    e=NormalizeException(write_registers(context,address,values,quantity));
    if (e!=MODBUS_EXCEPTION_NONE) return ModbusPdu_BuildException(MODBUS_FUNCTION_WRITE_MULTIPLE_REGISTERS,e,response_pdu,response_capacity,response_length);
    response_pdu[0]=MODBUS_FUNCTION_WRITE_MULTIPLE_REGISTERS; ModbusPdu_WriteU16(&response_pdu[1],address);
    ModbusPdu_WriteU16(&response_pdu[3],quantity); *response_length=MODBUS_FUNCTION_10_RESPONSE_LENGTH;
    return MODBUS_PDU_STATUS_OK;
}
