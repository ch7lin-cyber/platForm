#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include "HalAdcMeasurement.h"

int main(void)
{
    HalAdcConversionDiagnostics_t d;
    int32_t uv = 123;
    /* Production and diagnostic entry points must agree for both polarities,
     * invalid inputs, and int32 overflow, including unchanged failure output. */
    {
        const uint32_t raw[] = {0U, 0x800000UL, 0xFFFFFFUL, 0x1000000UL};
        const uint32_t reference[] = {0UL, 2500000UL, UINT32_MAX};
        const uint16_t gain[] = {0U, 1U, 32U, 128U};
        size_t r, v, g;
        unsigned int bipolar;
        for (r = 0U; r < sizeof(raw) / sizeof(raw[0]); r++)
        for (v = 0U; v < sizeof(reference) / sizeof(reference[0]); v++)
        for (g = 0U; g < sizeof(gain) / sizeof(gain[0]); g++)
        for (bipolar = 0U; bipolar < 2U; bipolar++)
        {
            int32_t production = 123, debug = 123;
            bool ok = HalAdcMeasurement_CodeToMicrovolts(
                raw[r], reference[v], gain[g], bipolar != 0U, &production);
            bool debug_ok = HalAdcMeasurement_CodeToMicrovoltsDiagnostic(
                raw[r], reference[v], gain[g], bipolar != 0U, &debug, &d);
            assert(ok == debug_ok && production == debug);
        }
        assert(!HalAdcMeasurement_CodeToMicrovolts(
            0U, 2500000UL, 32U, true, NULL));
    }
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
