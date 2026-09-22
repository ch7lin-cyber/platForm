#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "FactoryCalibrationService.h"
#include "HalAdcMeasurement.h"
#include "SensorConversionService.h"

static void TestCodeAndFactoryCalibration(void)
{
    HalAdcFactoryCalibration_t calibration = {-10L, 29990L, true};
    int32_t value;

    assert(HalAdcMeasurement_CodeToMicrovolts(
        0x00800000UL, 2500000UL, 32U, true, &value));
    assert(value == 0L);
    assert(HalAdcMeasurement_ApplyFactoryCalibration(
        14990L, &calibration, &value));
    assert(value == 15000L);
}

static void TestTcCjcPipeline(void)
{
    static const PiecewiseLinearSegment_t measurement_segment =
        {-100000L, 100000L, 25000L, 0LL};
    static const PiecewiseLinearSegment_t cjc_segment =
        {-200000L, 2000000L, 40L, 0LL};
    static const PiecewiseLinearTable_t measurement_table =
        {&measurement_segment, 1U, 1000UL};
    static const PiecewiseLinearTable_t cjc_table =
        {&cjc_segment, 1U, 1000UL};
    SensorConversionConfig_t config =
    {
        SENSOR_TYPE_TC_K, -200000L, 1300000L, 20000L,
        &measurement_table, &cjc_table, 0UL, 0UL, 1UL, 1UL
    };
    SensorConversionResult_t result;

    assert(SensorConversionService_Convert(&config, 1000L, 25000L,
                                           &result));
    assert(result.cjc_uv == 1000L);
    assert(result.physical_input == 2000L);
    assert(result.engineering_value == 50000L);
    assert(result.flags == 0UL);
}

static void TestRtdAndLimits(void)
{
    static const PiecewiseLinearSegment_t rtd_segment =
        {0L, 200000L, 2500L, -250000000LL};
    static const PiecewiseLinearTable_t rtd_table =
        {&rtd_segment, 1U, 1000UL};
    SensorConversionConfig_t config =
    {
        SENSOR_TYPE_RTD_PT100, -200000L, 850000L, 20000L,
        &rtd_table, NULL, 1000UL, 0UL, 1UL, 1UL
    };
    SensorConversionResult_t result;

    assert(SensorConversionService_Convert(&config, 100000L, 0L, &result));
    assert(result.physical_input == 100000L);
    assert(result.engineering_value == 0L);
    assert(result.flags == 0UL);

    config.nominal_max = -1000L;
    config.extended_margin = 20000L;
    assert(SensorConversionService_Convert(&config, 100000L, 0L, &result));
    assert((result.flags & SENSOR_FLAG_OUT_OF_SPEC) != 0UL);
    assert((result.flags & SENSOR_FLAG_OUT_OF_RANGE) == 0UL);
    config.nominal_max = -30000L;
    assert(SensorConversionService_Convert(&config, 100000L, 0L, &result));
    assert((result.flags & SENSOR_FLAG_OUT_OF_RANGE) != 0UL);
}

static void TestFactoryFlow(void)
{
    HalAdcFactoryCalibration_t calibration;
    FactoryCalibrationSnapshot_t snapshot;

    FactoryCalibrationService_Initialize();
    FactoryCalibrationService_SetUnlockKey1(FACTORY_CALIBRATION_UNLOCK_KEY);
    FactoryCalibrationService_SetUnlockKey2(FACTORY_CALIBRATION_UNLOCK_KEY);
    FactoryCalibrationService_UpdateLiveMicrovolts(5U, -4L);
    assert(FactoryCalibrationService_Select(5U,
                                            FACTORY_CAL_PROFILE_TC_GAIN1));
    assert(FactoryCalibrationService_CaptureZero());
    FactoryCalibrationService_UpdateLiveMicrovolts(5U, 30008L);
    assert(FactoryCalibrationService_CaptureSpan());
    assert(FactoryCalibrationService_Apply());
    assert(FactoryCalibrationService_GetCalibration(
        5U, FACTORY_CAL_PROFILE_TC_GAIN1, &calibration));
    assert(calibration.measured_zero_uv == -4L);
    assert(calibration.measured_span_uv == 30008L);
    FactoryCalibrationService_GetSnapshot(&snapshot);
    assert(snapshot.state == FACTORY_CAL_STATE_COMPLETE);
    assert(snapshot.revision == 1U);
}

int main(void)
{
    TestCodeAndFactoryCalibration();
    TestTcCjcPipeline();
    TestRtdAndLimits();
    TestFactoryFlow();
    return 0;
}
