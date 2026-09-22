#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "ModbusMaster.h"
#include "ModbusRtuFramer.h"

int main(void)
{
    const uint16_t write_values[] = {0x1234U, 0x5678U};
    ModbusMasterRequest_t request;
    ModbusExceptionCode_t exception;
    uint8_t request_adu[MODBUS_RTU_MAX_ADU_LENGTH];
    uint8_t response_adu[MODBUS_RTU_MAX_ADU_LENGTH];
    uint8_t response_pdu[] = {0x03U, 0x04U, 0x00U, 0x11U, 0x00U, 0x22U};
    uint16_t read_values[2] = {0U};
    size_t request_length;
    size_t response_length;
    size_t read_count;

    request.unit_address = 7U;
    request.function_code = 0x03U;
    request.starting_address = 0x0100U;
    request.quantity = 2U;
    request.write_values = NULL;
    assert(ModbusMaster_BuildRequest(
        SERIAL_PROTOCOL_MODBUS_RTU, &request,
        request_adu, sizeof(request_adu), &request_length) ==
           MODBUS_MASTER_STATUS_OK);
    assert(request_length == 8U);
    assert(ModbusRtuFramer_Encode(
        7U, response_pdu, sizeof(response_pdu),
        response_adu, sizeof(response_adu), &response_length) ==
           MODBUS_RTU_STATUS_OK);
    assert(ModbusMaster_ParseResponse(
        SERIAL_PROTOCOL_MODBUS_RTU, &request,
        response_adu, response_length,
        read_values, 2U, &read_count, &exception) ==
           MODBUS_MASTER_STATUS_OK);
    assert(read_count == 2U);
    assert(read_values[0] == 0x0011U);
    assert(read_values[1] == 0x0022U);

    request.function_code = 0x10U;
    request.starting_address = 0x0200U;
    request.quantity = 2U;
    request.write_values = write_values;
    assert(ModbusMaster_BuildRequest(
        SERIAL_PROTOCOL_MODBUS_ASCII, &request,
        request_adu, sizeof(request_adu), &request_length) ==
           MODBUS_MASTER_STATUS_OK);
    return 0;
}
