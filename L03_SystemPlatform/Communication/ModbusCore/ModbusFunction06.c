#include "ModbusFunction06.h"
#include <string.h>
static ModbusExceptionCode_t NormalizeException(ModbusExceptionCode_t e)
{
    switch(e) { case MODBUS_EXCEPTION_NONE: case MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS:
    case MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE: case MODBUS_EXCEPTION_SERVER_DEVICE_FAILURE:
    case MODBUS_EXCEPTION_SERVER_DEVICE_BUSY: return e; default: return MODBUS_EXCEPTION_SERVER_DEVICE_FAILURE; }
}
ModbusPduStatus_t ModbusFunction06_Process(const uint8_t *request_pdu,size_t request_length,
    ModbusFunction06WriteCallback_t write_register,void *context,uint8_t *response_pdu,
    size_t response_capacity,size_t *response_length)
{
    ModbusExceptionCode_t e; uint16_t address,value;
    if ((request_pdu==NULL)||(write_register==NULL)||(response_pdu==NULL)||(response_length==NULL)) return MODBUS_PDU_STATUS_INVALID_ARGUMENT;
    *response_length=0U;
    if ((request_length!=MODBUS_FUNCTION_06_REQUEST_LENGTH)||(request_pdu[0]!=MODBUS_FUNCTION_WRITE_SINGLE_REGISTER))
        return ModbusPdu_BuildException(MODBUS_FUNCTION_WRITE_SINGLE_REGISTER,MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE,response_pdu,response_capacity,response_length);
    if (response_capacity<MODBUS_FUNCTION_06_REQUEST_LENGTH) return MODBUS_PDU_STATUS_RESPONSE_TOO_SMALL;
    address=ModbusPdu_ReadU16(&request_pdu[1]); value=ModbusPdu_ReadU16(&request_pdu[3]);
    e=NormalizeException(write_register(context,address,value));
    if (e!=MODBUS_EXCEPTION_NONE) return ModbusPdu_BuildException(MODBUS_FUNCTION_WRITE_SINGLE_REGISTER,e,response_pdu,response_capacity,response_length);
    (void)memcpy(response_pdu,request_pdu,MODBUS_FUNCTION_06_REQUEST_LENGTH); *response_length=MODBUS_FUNCTION_06_REQUEST_LENGTH;
    return MODBUS_PDU_STATUS_OK;
}
