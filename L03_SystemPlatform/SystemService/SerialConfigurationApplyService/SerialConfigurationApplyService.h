#ifndef SERIAL_CONFIGURATION_APPLY_SERVICE_H
#define SERIAL_CONFIGURATION_APPLY_SERVICE_H

#include <stddef.h>
#include <stdint.h>

#include "ModbusRegisterAdapter.h"
#include "SerialService.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SERIAL_CONFIGURATION_APPLY_TIMEOUT_DEFAULT_MS (1000UL)

SerialServiceStatus_t SerialConfigurationApplyService_InitializePort(
    HalSerialPort_t port,
    const ModbusSerialPortConfiguration_t *configuration,
    uint32_t apply_response_timeout_ms,
    SerialServiceEventCallback_t event_callback,
    void *callback_context);
SerialServiceStatus_t SerialConfigurationApplyService_WriteResponse(
    HalSerialPort_t port,
    const uint8_t *response,
    size_t response_length);
void SerialConfigurationApplyService_Tick1ms(void);
void SerialConfigurationApplyService_Process(void);

#ifdef __cplusplus
}
#endif

#endif
