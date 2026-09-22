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
    int32_t microvolts;
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

typedef enum
{
    ANALOG_INPUT_SENSOR_DISABLED = 0,
    ANALOG_INPUT_SENSOR_THERMOCOUPLE,
    ANALOG_INPUT_SENSOR_RTD,
    ANALOG_INPUT_SENSOR_VOLTAGE,
    ANALOG_INPUT_SENSOR_CURRENT
} AnalogInputSensorClass_t;

typedef struct
{
    uint8_t logical_input;
    uint8_t device;
    uint8_t channel;
    AnalogInputSensorClass_t sensor_class;
} AnalogInputRoute_t;

AnalogInputStatus_t AnalogInputService_SetDeviceConfiguration(
    uint8_t device, const HalAdcDeviceConfig_t *config);
AnalogInputStatus_t AnalogInputService_SetRoutes(
    const AnalogInputRoute_t *routes, uint8_t route_count);
AnalogInputStatus_t AnalogInputService_Initialize(uint8_t device_count);
AnalogInputStatus_t AnalogInputService_Process(void);
AnalogInputStatus_t AnalogInputService_RetryDevice(uint8_t device);
bool AnalogInputService_GetLatest(uint8_t device,
                                  AnalogInputSample_t *sample);
bool AnalogInputService_GetDiagnostics(uint8_t device,
                                       AnalogInputDiagnostics_t *diagnostics);
bool AnalogInputService_GetLatestByInput(uint8_t logical_input,
                                         AnalogInputSample_t *sample);
bool AnalogInputService_GetRoute(uint8_t logical_input,
                                 AnalogInputRoute_t *route);

#ifdef __cplusplus
}
#endif

#endif
