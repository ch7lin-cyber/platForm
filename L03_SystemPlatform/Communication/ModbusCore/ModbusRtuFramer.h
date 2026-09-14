#ifndef MODBUS_RTU_FRAMER_H
#define MODBUS_RTU_FRAMER_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MODBUS_RTU_MIN_ADU_LENGTH (4U)
#define MODBUS_RTU_MAX_ADU_LENGTH (256U)
#define MODBUS_RTU_MAX_PDU_LENGTH (253U)

typedef enum
{
    MODBUS_RTU_STATUS_OK = 0,
    MODBUS_RTU_STATUS_INVALID_ARGUMENT,
    MODBUS_RTU_STATUS_FRAME_TOO_SHORT,
    MODBUS_RTU_STATUS_FRAME_TOO_LONG,
    MODBUS_RTU_STATUS_CRC_ERROR,
    MODBUS_RTU_STATUS_RESPONSE_TOO_SMALL
} ModbusRtuStatus_t;

typedef struct
{
    uint8_t address;
    const uint8_t *pdu;
    size_t pdu_length;
} ModbusRtuRequestView_t;

ModbusRtuStatus_t ModbusRtuFramer_Decode(
    const uint8_t *adu,
    size_t adu_length,
    ModbusRtuRequestView_t *request);

ModbusRtuStatus_t ModbusRtuFramer_Encode(
    uint8_t address,
    const uint8_t *pdu,
    size_t pdu_length,
    uint8_t *adu,
    size_t adu_capacity,
    size_t *adu_length);

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_RTU_FRAMER_H */
