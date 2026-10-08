#ifndef FACTORY_CALIBRATION_SERVICE_H
#define FACTORY_CALIBRATION_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

#include "HalAdcMeasurement.h"
#include "FactoryModeService.h"

#ifndef FACTORY_CALIBRATION_INPUT_COUNT
#define FACTORY_CALIBRATION_INPUT_COUNT   (16U)
#endif
#if (FACTORY_CALIBRATION_INPUT_COUNT < 1U) || (FACTORY_CALIBRATION_INPUT_COUNT > 16U)
#error "FACTORY_CALIBRATION_INPUT_COUNT must be 1..16"
#endif
#define FACTORY_CALIBRATION_PROFILE_COUNT (10U)
#define FACTORY_CALIBRATION_UNLOCK_KEY     FACTORY_MODE_UNLOCK_KEY
typedef enum
{
    FACTORY_CAL_PROFILE_TC_GAIN32 = 0,
    FACTORY_CAL_PROFILE_TC_GAIN64 = 1,
    FACTORY_CAL_PROFILE_RTD_GAIN8 = 2,
    FACTORY_CAL_PROFILE_TC_GAIN16 = 3,
    FACTORY_CAL_PROFILE_VOLTAGE_GAIN32 = 4,
    FACTORY_CAL_PROFILE_RTD_GAIN16 = 5,
    FACTORY_CAL_PROFILE_CURRENT_GAIN64 = 6,
    FACTORY_CAL_PROFILE_TC_GAIN128 = 7,
    FACTORY_CAL_PROFILE_RTD_GAIN1 = 8,
    FACTORY_CAL_PROFILE_RTD_GAIN32 = 9,
    /* Source aliases; external tools must use the explicit profile IDs above. */
    FACTORY_CAL_PROFILE_TC_GAIN1 = FACTORY_CAL_PROFILE_TC_GAIN32,
    FACTORY_CAL_PROFILE_TC_GAIN2 = FACTORY_CAL_PROFILE_TC_GAIN64,
    FACTORY_CAL_PROFILE_RTD = FACTORY_CAL_PROFILE_RTD_GAIN8,
    FACTORY_CAL_PROFILE_MV50 = FACTORY_CAL_PROFILE_TC_GAIN32,
    FACTORY_CAL_PROFILE_V5 = FACTORY_CAL_PROFILE_VOLTAGE_GAIN32,
    FACTORY_CAL_PROFILE_V10 = FACTORY_CAL_PROFILE_VOLTAGE_GAIN32,
    FACTORY_CAL_PROFILE_MA = FACTORY_CAL_PROFILE_CURRENT_GAIN64
} FactoryCalibrationProfile_t;

typedef enum
{
    FACTORY_CAL_STATE_LOCKED = 0,
    FACTORY_CAL_STATE_READY,
    FACTORY_CAL_STATE_ZERO_CAPTURED,
    FACTORY_CAL_STATE_SPAN_CAPTURED,
    FACTORY_CAL_STATE_COMPLETE,
    FACTORY_CAL_STATE_ERROR
} FactoryCalibrationState_t;

typedef enum
{
    FACTORY_CAL_ERROR_NONE = 0,
    FACTORY_CAL_ERROR_INVALID_ARGUMENT,
    FACTORY_CAL_ERROR_LOCKED,
    FACTORY_CAL_ERROR_NO_LIVE_SAMPLE,
    FACTORY_CAL_ERROR_SEQUENCE,
    FACTORY_CAL_ERROR_SPAN_TOO_SMALL
} FactoryCalibrationError_t;

typedef struct
{
    uint8_t input;
    FactoryCalibrationProfile_t profile;
    FactoryCalibrationState_t state;
    FactoryCalibrationError_t error;
    int32_t live_uv;
    int32_t pending_zero_uv;
    int32_t pending_span_uv;
    uint16_t revision;
    bool live_valid;
} FactoryCalibrationSnapshot_t;

void FactoryCalibrationService_Initialize(void);
void FactoryCalibrationService_SetUnlockKey1(uint16_t key);
void FactoryCalibrationService_SetUnlockKey2(uint16_t key);
bool FactoryCalibrationService_Select(uint8_t input,
                                      FactoryCalibrationProfile_t profile);
void FactoryCalibrationService_UpdateLiveMicrovolts(uint8_t input,
                                                     int32_t microvolts);
bool FactoryCalibrationService_CaptureZero(void);
bool FactoryCalibrationService_CaptureSpan(void);
bool FactoryCalibrationService_Apply(void);
void FactoryCalibrationService_Abort(void);
bool FactoryCalibrationService_GetCalibration(
    uint8_t input, FactoryCalibrationProfile_t profile,
    HalAdcFactoryCalibration_t *calibration);
/* False for identity defaults; true only after a successful Apply. */
bool FactoryCalibrationService_IsCalibrated(
    uint8_t input, FactoryCalibrationProfile_t profile);
void FactoryCalibrationService_GetSnapshot(
    FactoryCalibrationSnapshot_t *snapshot);

#endif
