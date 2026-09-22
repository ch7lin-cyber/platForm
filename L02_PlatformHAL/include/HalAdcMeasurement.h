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

bool HalAdcMeasurement_CodeToMicrovolts(uint32_t raw_code,
                                        uint32_t reference_uv,
                                        uint16_t gain,
                                        bool bipolar,
                                        int32_t *microvolts);
bool HalAdcMeasurement_ApplyFactoryCalibration(
    int32_t uncalibrated_uv,
    const HalAdcFactoryCalibration_t *calibration,
    int32_t *calibrated_uv);
bool HalAdcMeasurement_ValidateFactoryCalibration(
    const HalAdcFactoryCalibration_t *calibration);

#ifdef __cplusplus
}
#endif

#endif
