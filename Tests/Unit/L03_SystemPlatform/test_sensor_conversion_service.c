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
        &measurement_table, &cjc_table, 0UL, 0UL, 1UL, 1UL, false
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
        &rtd_table, NULL, 1000UL, 0UL, 1UL, 1UL, false
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

static void TestVoltageSamplesAndBounds(void)
{
    SensorConversionConfig_t config =
    {
        SENSOR_TYPE_VOLTAGE_0_5V, -32000L, 32000L, 0L,
        NULL, NULL, 0UL, 0UL, 1026200UL, 6200UL, false
    };
    SensorConversionResult_t result;

    assert(SensorConversionService_Convert(&config, 5698L, 0L, &result));
    assert(result.physical_input == 943110L);
    assert(result.engineering_value == -19929L);
    assert(SensorConversionService_Convert(&config, 28610L, 0L, &result));
    assert(result.physical_input == 4735416L);
    assert(result.engineering_value == 28613L);

    config.frontend_ratio_numerator = 1UL;
    config.frontend_ratio_denominator = 1UL;
    assert(SensorConversionService_Convert(&config, -2147483647L - 1L,
                                           0L, &result));
    assert(result.physical_input == (-2147483647L - 1L));
    assert(SensorConversionService_Convert(&config, 2147483647L,
                                           0L, &result));
    assert(result.physical_input == 2147483647L);
    config.frontend_ratio_numerator = 2UL;
    assert(!SensorConversionService_Convert(&config, 2147483647L,
                                            0L, &result));
    assert(!SensorConversionService_Convert(&config, -2147483647L - 1L,
                                            0L, &result));
}

static void TestTableSignedBounds(void)
{
    PiecewiseLinearSegment_t segment = {0L, 0L, 0L, -2147483647LL - 1LL};
    PiecewiseLinearTable_t table = {&segment, 1U, 1UL};
    int32_t value;

    assert(PiecewiseLinearTable_Evaluate(&table, 0L, &value));
    assert(value == (-2147483647L - 1L));
    segment.intercept = 2147483647LL;
    assert(PiecewiseLinearTable_Evaluate(&table, 0L, &value));
    assert(value == 2147483647L);
    segment.intercept = 2147483648LL;
    assert(!PiecewiseLinearTable_Evaluate(&table, 0L, &value));
    segment.intercept = -2147483649LL;
    assert(!PiecewiseLinearTable_Evaluate(&table, 0L, &value));
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

static void TestNominalAndExtendedEndpoints(void)
{
    /* mC = input*1000; table only covers the nominal 0..100 C domain. */
    const PiecewiseLinearSegment_t segment = {0L, 100L, 1000L, 0LL};
    const PiecewiseLinearTable_t table = {&segment, 1U, 1UL};
    SensorConversionConfig_t config = {0};
    SensorConversionResult_t result;
    const int32_t inputs[] = {-21L, -20L, -1L, 0L, 100L, 101L, 120L, 121L};
    const uint32_t flags[] = {3UL, 1UL, 1UL, 0UL, 0UL, 1UL, 1UL, 3UL};
    config.type = SENSOR_TYPE_TC_K;
    config.nominal_min = 0L;
    config.nominal_max = 100000L;
    config.extended_margin = 20000L;
    config.measurement_table = &table;
    config.cold_junction_compensation_disabled = true;
    for (unsigned int i = 0U; i < sizeof(inputs)/sizeof(inputs[0]); i++)
    {
        assert(SensorConversionService_Convert(&config, inputs[i], 0L, &result));
        assert(result.engineering_value == inputs[i]*1000L);
        assert(result.flags == flags[i]);
    }
    config.type = SENSOR_TYPE_RTD_PT100;
    config.excitation_current_ua = 1000UL;
    for (unsigned int i = 0U; i < sizeof(inputs)/sizeof(inputs[0]); i++)
    {
        assert(SensorConversionService_Convert(&config, inputs[i], 0L, &result));
        assert(result.flags == flags[i]);
    }
}

static void TestAnalogPhysicalLimits(void)
{
    const SensorConversionType_t types[] = {
        SENSOR_TYPE_VOLTAGE_0_5V, SENSOR_TYPE_VOLTAGE_0_10V,
        SENSOR_TYPE_VOLTAGE_0_50MV, SENSOR_TYPE_CURRENT_0_20MA,
        SENSOR_TYPE_CURRENT_4_20MA};
    const int32_t low[] = {0L, 0L, 0L, 0L, 4000L};
    const int32_t high[] = {5000000L, 10000000L, 50000L, 20000L, 20000L};
    SensorConversionConfig_t config = {0};
    SensorConversionResult_t result;
    config.frontend_ratio_numerator = 1UL;
    config.frontend_ratio_denominator = 1UL;
    config.shunt_resistance_milliohm = 1000UL; /* test fixture: uV == uA */
    /* Deliberately unrelated engineering limits must not affect analog flags. */
    config.nominal_min = 123L;
    config.nominal_max = 124L;
    for (unsigned int i = 0U; i < 5U; i++)
    {
        const int32_t margin = (high[i] - low[i]) / 50L;
        const int32_t points[] = {low[i]-margin-1L, low[i]-margin,
            low[i]-1L, low[i], high[i], high[i]+1L,
            high[i]+margin, high[i]+margin+1L};
        const uint32_t flags[] = {3UL, 1UL, 1UL, 0UL, 0UL, 1UL, 1UL, 3UL};
        config.type = types[i];
        for (unsigned int j = 0U; j < 8U; j++)
        {
            assert(SensorConversionService_Convert(&config, points[j], 0L, &result));
            assert(result.flags == flags[j]);
        }
    }
    /* Real 49.9 ohm shunt: do not truncate a fractional overrange uA. */
    config.type = SENSOR_TYPE_CURRENT_4_20MA;
    config.shunt_resistance_milliohm = 49900UL;
    assert(SensorConversionService_Convert(&config, 1013968L, 0L, &result));
    assert(result.flags == SENSOR_FLAG_OUT_OF_SPEC); /* exactly 20.32 mA */
    assert(SensorConversionService_Convert(&config, 1013969L, 0L, &result));
    assert(result.flags == (SENSOR_FLAG_OUT_OF_SPEC | SENSOR_FLAG_OUT_OF_RANGE));
}

int main(void)
{
    TestCodeAndFactoryCalibration();
    TestTcCjcPipeline();
    TestRtdAndLimits();
    TestVoltageSamplesAndBounds();
    TestTableSignedBounds();
    TestFactoryFlow();
    TestNominalAndExtendedEndpoints();
    TestAnalogPhysicalLimits();
    return 0;
}

