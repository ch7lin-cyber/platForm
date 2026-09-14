#ifndef MODBUS_LRC_H
#define MODBUS_LRC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t ModbusLrc_Calculate(const uint8_t *data, size_t length);
bool ModbusLrc_IsFrameValid(const uint8_t *data_with_lrc, size_t length);
bool ModbusLrc_Append(uint8_t *frame, size_t payload_length, size_t capacity);

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_LRC_H */
