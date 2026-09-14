#ifndef MODBUS_CRC16_H
#define MODBUS_CRC16_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint16_t ModbusCrc16_Calculate(const uint8_t *data, size_t length);
bool ModbusCrc16_IsFrameValid(const uint8_t *frame, size_t length);
bool ModbusCrc16_Append(uint8_t *frame, size_t payload_length, size_t capacity);

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_CRC16_H */
