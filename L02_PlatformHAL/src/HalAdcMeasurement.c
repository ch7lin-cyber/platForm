#include "HalAdcMeasurement.h"

#include <limits.h>
#include <stddef.h>

#define ADC_24BIT_HALF_SCALE (8388608LL)
#define ADC_24BIT_FULL_SCALE (16777216LL)
#define MIN_FACTORY_SPAN_UV  (1000L)

static bool StoreInt32(int64_t value, int32_t *result)
{
    if ((result == NULL) || (value < INT32_MIN) || (value > INT32_MAX))
    {
        return false;
    }
    *result = (int32_t)value;
    return true;
}

bool HalAdcMeasurement_CodeToMicrovoltsDiagnostic(
    uint32_t raw_code, uint32_t reference_uv, uint16_t gain, bool bipolar,
    int32_t *microvolts, HalAdcConversionDiagnostics_t *diagnostics)
{
    int64_t numerator;
    int64_t denominator;
    int64_t quotient;
    HalAdcConversionResult_t result = HAL_ADC_CONVERSION_OK;

    if (diagnostics != NULL)
    {
        diagnostics->result = HAL_ADC_CONVERSION_NONE;
        diagnostics->raw_code = raw_code;
        diagnostics->reference_uv = reference_uv;
        diagnostics->gain = gain;
        diagnostics->bipolar = bipolar;
        diagnostics->output_valid = (microvolts != NULL);
        diagnostics->numerator = 0;
        diagnostics->denominator = 0;
        diagnostics->quotient = 0;
        diagnostics->minimum = (int64_t)INT32_MIN;
        diagnostics->maximum = (int64_t)INT32_MAX;
    }
    if (raw_code > 0x00FFFFFFUL)
    {
        result = HAL_ADC_CONVERSION_RAW_INVALID;
    }
    else if (reference_uv == 0UL)
    {
        result = HAL_ADC_CONVERSION_REFERENCE_ZERO;
    }
    else if (gain == 0U)
    {
        result = HAL_ADC_CONVERSION_GAIN_ZERO;
    }
    else if (microvolts == NULL)
    {
        result = HAL_ADC_CONVERSION_OUTPUT_NULL;
    }
    if (result != HAL_ADC_CONVERSION_OK)
    {
        if (diagnostics != NULL)
        {
            diagnostics->result = result;
        }
        return false;
    }

    if (bipolar)
    {
        numerator = ((int64_t)raw_code - ADC_24BIT_HALF_SCALE) *
                    (int64_t)reference_uv;
        denominator = ADC_24BIT_HALF_SCALE * (int64_t)gain;
    }
    else
    {
        numerator = (int64_t)raw_code * (int64_t)reference_uv;
        denominator = ADC_24BIT_FULL_SCALE * (int64_t)gain;
    }
    quotient = numerator / denominator;
    if (diagnostics != NULL)
    {
        diagnostics->numerator = numerator;
        diagnostics->denominator = denominator;
        diagnostics->quotient = quotient;
    }
    if (!StoreInt32(quotient, microvolts))
    {
        result = HAL_ADC_CONVERSION_OUT_OF_RANGE;
    }
    if (diagnostics != NULL)
    {
        diagnostics->result = result;
    }
    return result == HAL_ADC_CONVERSION_OK;
}

bool HalAdcMeasurement_CodeToMicrovolts(uint32_t raw_code,
                                        uint32_t reference_uv,
                                        uint16_t gain,
                                        bool bipolar,
                                        int32_t *microvolts)
{
    return HalAdcMeasurement_CodeToMicrovoltsDiagnostic(
        raw_code, reference_uv, gain, bipolar, microvolts, NULL);
}

bool HalAdcMeasurement_ValidateFactoryCalibration(
    const HalAdcFactoryCalibration_t *calibration)
{
    int64_t measured_span;

    if ((calibration == NULL) || !calibration->valid)
    {
        return false;
    }
    measured_span = (int64_t)calibration->measured_span_uv -
                    (int64_t)calibration->measured_zero_uv;
    return (measured_span >= MIN_FACTORY_SPAN_UV) ||
           (measured_span <= -MIN_FACTORY_SPAN_UV);
}

bool HalAdcMeasurement_ApplyFactoryCalibration(
    int32_t uncalibrated_uv,
    const HalAdcFactoryCalibration_t *calibration,
    int32_t *calibrated_uv)
{
    int64_t numerator;
    int64_t denominator;

    if (!HalAdcMeasurement_ValidateFactoryCalibration(calibration) ||
        (calibrated_uv == NULL))
    {
        return false;
    }
    numerator = ((int64_t)uncalibrated_uv -
                 (int64_t)calibration->measured_zero_uv) *
                HAL_ADC_FACTORY_SPAN_UV;
    denominator = (int64_t)calibration->measured_span_uv -
                  (int64_t)calibration->measured_zero_uv;
    return StoreInt32(numerator / denominator, calibrated_uv);
}
