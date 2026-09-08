#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "ModbusFunction06.h"
#include "ModbusFunction10.h"

typedef struct { uint16_t address; uint16_t values[4]; uint16_t quantity; ModbusExceptionCode_t result; } FakeRegisters_t;
static ModbusExceptionCode_t WriteSingle(void *context,uint16_t address,uint16_t value)
{
    FakeRegisters_t *fake=(FakeRegisters_t *)context;
    if (fake->result!=MODBUS_EXCEPTION_NONE) return fake->result;
    fake->address=address; fake->values[0]=value; fake->quantity=1U; return MODBUS_EXCEPTION_NONE;
}
static ModbusExceptionCode_t WriteMultiple(void *context,uint16_t address,const uint16_t *values,uint16_t quantity)
{
    FakeRegisters_t *fake=(FakeRegisters_t *)context;
    if (fake->result!=MODBUS_EXCEPTION_NONE) return fake->result;
    assert(quantity<=4U); fake->address=address; fake->quantity=quantity;
    (void)memcpy(fake->values,values,(size_t)quantity*sizeof(values[0])); return MODBUS_EXCEPTION_NONE;
}
int main(void)
{
    const uint8_t request06[]={0x06U,0x11U,0x03U,0x00U,0x0EU};
    const uint8_t request10[]={0x10U,0x11U,0x20U,0x00U,0x02U,0x04U,0x00U,0x01U,0x12U,0x34U};
    const uint8_t invalid10[]={0x10U,0x11U,0x20U,0x00U,0x02U,0x02U,0x00U,0x01U};
    uint8_t response[16]={0}; size_t length=0U; FakeRegisters_t fake={0};

    assert(ModbusFunction06_Process(request06,sizeof(request06),WriteSingle,&fake,response,sizeof(response),&length)==MODBUS_PDU_STATUS_OK);
    assert(length==sizeof(request06)); assert(memcmp(request06,response,length)==0);
    assert(fake.address==0x1103U); assert(fake.values[0]==0x000EU);

    fake.result=MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    assert(ModbusFunction06_Process(request06,sizeof(request06),WriteSingle,&fake,response,sizeof(response),&length)==MODBUS_PDU_STATUS_OK);
    assert((length==2U)&&(response[0]==0x86U)&&(response[1]==0x02U)); fake.result=MODBUS_EXCEPTION_NONE;

    assert(ModbusFunction10_Process(request10,sizeof(request10),WriteMultiple,&fake,response,sizeof(response),&length)==MODBUS_PDU_STATUS_OK);
    assert((length==5U)&&(response[0]==0x10U)); assert(fake.address==0x1120U);
    assert((fake.quantity==2U)&&(fake.values[0]==1U)&&(fake.values[1]==0x1234U));

    assert(ModbusFunction10_Process(invalid10,sizeof(invalid10),WriteMultiple,&fake,response,sizeof(response),&length)==MODBUS_PDU_STATUS_OK);
    assert((length==2U)&&(response[0]==0x90U)&&(response[1]==0x03U));
    return 0;
}
