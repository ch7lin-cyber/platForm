#ifndef MODBUS_ASCII_FRAMER_H
#define MODBUS_ASCII_FRAMER_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MODBUS_ASCII_START_CHARACTER     (0x3AU)
#define MODBUS_ASCII_CR_CHARACTER        (0x0DU)
#define MODBUS_ASCII_LF_CHARACTER        (0x0AU)
#define MODBUS_ASCII_MIN_ADU_LENGTH      (9U)
#define MODBUS_ASCII_MAX_ADU_LENGTH      (513U)
#define MODBUS_ASCII_MAX_PDU_LENGTH      (253U)
#define MODBUS_ASCII_MAX_BINARY_LENGTH   (255U)

typedef enum
{
    MODBUS_ASCII_STATUS_OK = 0,
    MODBUS_ASCII_STATUS_INVALID_ARGUMENT,
    MODBUS_ASCII_STATUS_FRAME_TOO_SHORT,
    MODBUS_ASCII_STATUS_FRAME_TOO_LONG,
    MODBUS_ASCII_STATUS_MISSING_START,
    MODBUS_ASCII_STATUS_MISSING_CRLF,
    MODBUS_ASCII_STATUS_ODD_HEX_LENGTH,
    MODBUS_ASCII_STATUS_INVALID_HEX,
    MODBUS_ASCII_STATUS_LRC_ERROR,
    MODBUS_ASCII_STATUS_BUFFER_TOO_SMALL
} ModbusAsciiStatus_t;

typedef struct
{
    uint8_t address;
    const uint8_t *pdu;
    size_t pdu_length;
} ModbusAsciiRequestView_t;

ModbusAsciiStatus_t ModbusAsciiFramer_Decode(
    const uint8_t *adu,
    size_t adu_length,
    uint8_t *decode_buffer,
    size_t decode_capacity,
    ModbusAsciiRequestView_t *request);

ModbusAsciiStatus_t ModbusAsciiFramer_Encode(
    uint8_t address,
    const uint8_t *pdu,
    size_t pdu_length,
    uint8_t *adu,
    size_t adu_capacity,
    size_t *adu_length);

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_ASCII_FRAMER_H */
