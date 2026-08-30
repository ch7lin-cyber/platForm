#ifndef SERIAL_CONFIGURATION_H
#define SERIAL_CONFIGURATION_H

#include <stdbool.h>
#include <stdint.h>

#include "HalSerial.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    SERIAL_PROTOCOL_RAW = 0,
    SERIAL_PROTOCOL_MODBUS_RTU,
    SERIAL_PROTOCOL_MODBUS_ASCII
} SerialProtocol_t;

typedef enum
{
    SERIAL_ROLE_GENERIC = 0,
    SERIAL_ROLE_MODBUS_SLAVE,
    SERIAL_ROLE_MODBUS_MASTER
} SerialRole_t;

typedef struct
{
    HalSerialConfig_t line;
    SerialProtocol_t protocol;
    SerialRole_t role;
    uint32_t response_timeout_ms;
} SerialConfiguration_t;

void SerialConfiguration_SetRtuDefault(SerialConfiguration_t *configuration,
                                       SerialRole_t role);
void SerialConfiguration_SetAsciiDefault(SerialConfiguration_t *configuration,
                                         SerialRole_t role);
bool SerialConfiguration_IsValid(const SerialConfiguration_t *configuration);
uint32_t SerialConfiguration_GetCharacterTimeUs(
    const SerialConfiguration_t *configuration);
uint32_t SerialConfiguration_GetRtuFrameGapUs(
    const SerialConfiguration_t *configuration);

#ifdef __cplusplus
}
#endif

#endif
