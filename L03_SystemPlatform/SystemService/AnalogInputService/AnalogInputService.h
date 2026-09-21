#ifndef ANALOG_INPUT_SERVICE_H
#define ANALOG_INPUT_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

#include "HalAdc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    ANALOG_INPUT_STATUS_OK = 0,
    ANALOG_INPUT_STATUS_INVALID_ARGUMENT,
    ANALOG_INPUT_STATUS_NOT_INITIALIZED,
    ANALOG_INPUT_STATUS_NOT_READY,
    ANALOG_INPUT_STATUS_DRIVER_ERROR
} AnalogInputStatus_t;

typedef struct
{
    uint32_t raw_code;
    uint32_t sequence;
    uint8_t channel;
    bool valid;
} AnalogInputSample_t;

typedef struct
{
    uint32_t successful_samples;
    uint32_t not_ready_polls;
    uint32_t driver_errors;
    bool online;
} AnalogInputDiagnostics_t;

AnalogInputStatus_t AnalogInputService_Initialize(uint8_t device_count);
AnalogInputStatus_t AnalogInputService_Process(void);
AnalogInputStatus_t AnalogInputService_RetryDevice(uint8_t device);
bool AnalogInputService_GetLatest(uint8_t device,
                                  AnalogInputSample_t *sample);
bool AnalogInputService_GetDiagnostics(uint8_t device,
                                       AnalogInputDiagnostics_t *diagnostics);

#ifdef __cplusplus
}
#endif

#endif
