#ifndef HAL_ADC_MEASUREMENT_H
#define HAL_ADC_MEASUREMENT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HAL_ADC_FACTORY_ZERO_UV (0L)
#define HAL_ADC_FACTORY_SPAN_UV (30000L)

typedef struct
{
    int32_t measured_zero_uv;
    int32_t measured_span_uv;
    bool valid;
} HalAdcFactoryCalibration_t;

typedef enum
{
    HAL_ADC_CONVERSION_NONE = 0,
    HAL_ADC_CONVERSION_OK,
    HAL_ADC_CONVERSION_RAW_INVALID,
    HAL_ADC_CONVERSION_REFERENCE_ZERO,
    HAL_ADC_CONVERSION_GAIN_ZERO,
    HAL_ADC_CONVERSION_OUTPUT_NULL,
    HAL_ADC_CONVERSION_OUT_OF_RANGE
} HalAdcConversionResult_t;

typedef struct
{
    HalAdcConversionResult_t result;
    uint32_t raw_code;
    uint32_t reference_uv;
    uint16_t gain;
    bool bipolar;
    bool output_valid;
    int64_t numerator;
    int64_t denominator;
    int64_t quotient;
    int64_t minimum;
    int64_t maximum;
} HalAdcConversionDiagnostics_t;

/* Optional per-call diagnostics; caller owns storage (no shared global state). */
bool HalAdcMeasurement_CodeToMicrovoltsDiagnostic(
    uint32_t raw_code, uint32_t reference_uv, uint16_t gain, bool bipolar,
    int32_t *microvolts, HalAdcConversionDiagnostics_t *diagnostics);

bool HalAdcMeasurement_CodeToMicrovolts(uint32_t raw_code,
                                        uint32_t reference_uv,
                                        uint16_t gain,
                                        bool bipolar,
                                        int32_t *microvolts);
bool HalAdcMeasurement_ApplyFactoryCalibration(
    int32_t uncalibrated_uv,
    const HalAdcFactoryCalibration_t *calibration,
    int32_t *calibrated_uv);
bool HalAdcMeasurement_ApplyFactoryCalibrationTargets(
    int32_t uncalibrated_uv, const HalAdcFactoryCalibration_t *calibration,
    int32_t target_zero_uv, int32_t target_span_uv, int32_t *calibrated_uv);
bool HalAdcMeasurement_ValidateFactoryCalibration(
    const HalAdcFactoryCalibration_t *calibration);

#ifdef __cplusplus
}
#endif

#endif
