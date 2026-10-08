#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include "HalAdcMeasurement.h"

int main(void)
{
    HalAdcConversionDiagnostics_t d;
    int32_t uv = 123;
    assert(HalAdcMeasurement_CodeToMicrovoltsDiagnostic(
        0x8955DDUL, 2500000UL, 32U, true, &uv, &d));
    assert(uv == 5697 && d.result == HAL_ADC_CONVERSION_OK);
    assert(d.raw_code == 0x8955DDUL && d.gain == 32U);
    assert(d.numerator == (int64_t)(0x8955DDUL - 0x800000UL) * 2500000LL);
    assert(d.denominator == 268435456LL && d.quotient == 5697);
    assert(d.minimum == -2147483648LL && d.maximum == 2147483647LL);
    assert(d.output_valid && d.bipolar);
    assert(HalAdcMeasurement_CodeToMicrovoltsDiagnostic(
        0xAED3F0UL, 2500000UL, 32U, true, &uv, &d));
    assert(uv == 28581);
    assert(HalAdcMeasurement_CodeToMicrovoltsDiagnostic(
        0U, 2500000UL, 32U, true, &uv, &d));
    assert(uv == -78125 && d.quotient == -78125);
    assert(HalAdcMeasurement_CodeToMicrovoltsDiagnostic(
        0x800000UL, 2500000UL, 32U, false, &uv, &d));
    assert(uv == 39062 && !d.bipolar);
    assert(!HalAdcMeasurement_CodeToMicrovoltsDiagnostic(
        0x1000000UL, 2500000UL, 32U, true, &uv, &d));
    assert(d.result == HAL_ADC_CONVERSION_RAW_INVALID && d.denominator == 0);
    assert(!HalAdcMeasurement_CodeToMicrovoltsDiagnostic(
        0U, 0U, 32U, true, &uv, &d));
    assert(d.result == HAL_ADC_CONVERSION_REFERENCE_ZERO);
    assert(!HalAdcMeasurement_CodeToMicrovoltsDiagnostic(
        0U, 2500000UL, 0U, true, &uv, &d));
    assert(d.result == HAL_ADC_CONVERSION_GAIN_ZERO);
    assert(!HalAdcMeasurement_CodeToMicrovoltsDiagnostic(
        0U, 2500000UL, 32U, true, NULL, &d));
    assert(d.result == HAL_ADC_CONVERSION_OUTPUT_NULL && !d.output_valid);
    uv = 123;
    assert(!HalAdcMeasurement_CodeToMicrovoltsDiagnostic(
        0xFFFFFFUL, UINT32_MAX, 1U, false, &uv, &d));
    assert(d.result == HAL_ADC_CONVERSION_OUT_OF_RANGE);
    assert(d.quotient > INT32_MAX && uv == 123);
    assert(HalAdcMeasurement_CodeToMicrovolts(0x8955DDUL, 2500000UL, 32U, true, &uv));
    assert(uv == 5697);
    assert(HalAdcMeasurement_CodeToMicrovoltsDiagnostic(
        0x8948CCUL, 2500000UL, 32U, true, &uv, &d));
    assert(uv == 5666);
    assert(HalAdcMeasurement_CodeToMicrovoltsDiagnostic(
        0xAED42EUL, 2500000UL, 32U, true, &uv, &d));
    assert(uv == 28582);
    {
        HalAdcFactoryCalibration_t calibration = {0, 30000, true};
        assert(HalAdcMeasurement_ApplyFactoryCalibration(
            (int32_t)(-2147483647LL - 1LL), &calibration, &uv));
        assert((int64_t)uv == -2147483648LL);
        assert(HalAdcMeasurement_ApplyFactoryCalibration(
            (int32_t)2147483647LL, &calibration, &uv));
        assert((int64_t)uv == 2147483647LL);
        calibration.measured_span_uv = 1000;
        uv = 123;
        assert(!HalAdcMeasurement_ApplyFactoryCalibration(
            (int32_t)(-2147483647LL - 1LL), &calibration, &uv));
        assert(uv == 123);
        assert(!HalAdcMeasurement_ApplyFactoryCalibration(
            (int32_t)2147483647LL, &calibration, &uv));
        assert(uv == 123);
    }
    return 0;
}
