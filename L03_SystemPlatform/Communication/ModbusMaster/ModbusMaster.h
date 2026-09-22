#ifndef MODBUS_MASTER_H
#define MODBUS_MASTER_H

#include <stddef.h>
#include <stdint.h>

#include "ModbusPdu.h"
#include "SerialConfiguration.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MODBUS_MASTER_READ_QUANTITY_MAX  (125U)
#define MODBUS_MASTER_WRITE_QUANTITY_MAX (123U)

typedef enum
{
    MODBUS_MASTER_STATUS_OK = 0,
    MODBUS_MASTER_STATUS_INVALID_ARGUMENT,
    MODBUS_MASTER_STATUS_BUFFER_TOO_SMALL,
    MODBUS_MASTER_STATUS_FRAME_ERROR,
    MODBUS_MASTER_STATUS_ADDRESS_MISMATCH,
    MODBUS_MASTER_STATUS_FUNCTION_MISMATCH,
    MODBUS_MASTER_STATUS_EXCEPTION_RESPONSE,
    MODBUS_MASTER_STATUS_DATA_MISMATCH,
    MODBUS_MASTER_STATUS_TIMEOUT
} ModbusMasterStatus_t;

typedef struct
{
    uint8_t unit_address;
    uint8_t function_code;
    uint16_t starting_address;
    uint16_t quantity;
    const uint16_t *write_values;
} ModbusMasterRequest_t;

ModbusMasterStatus_t ModbusMaster_BuildRequest(
    SerialProtocol_t protocol,
    const ModbusMasterRequest_t *request,
    uint8_t *adu,
    size_t adu_capacity,
    size_t *adu_length);

ModbusMasterStatus_t ModbusMaster_ParseResponse(
    SerialProtocol_t protocol,
    const ModbusMasterRequest_t *request,
    const uint8_t *adu,
    size_t adu_length,
    uint16_t *read_values,
    size_t read_capacity,
    size_t *read_count,
    ModbusExceptionCode_t *exception);

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_MASTER_H */
