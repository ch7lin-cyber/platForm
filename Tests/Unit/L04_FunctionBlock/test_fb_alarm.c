#include "FbAlarm.h"

#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static FbAlarmConfig_t Config(FbAlarmMode_t mode)
{
    FbAlarmConfig_t config = {0};
    FbAlarm_DefaultConfig(&config);
    config.mode = mode;
    config.alarm_high = 100;
    config.alarm_low = 20;
    return config;
}

static FbAlarmInput_t Input(void)
{
    FbAlarmInput_t input = {0};
    input.enable = true;
    input.valid = true;
    input.setpoint = 1000;
    return input;
}

static FbAlarmOutput_t Step(FbAlarm_t *fb, const FbAlarmConfig_t *config,
                             FbAlarmInput_t *input, int32_t pv, uint32_t ms)
{
    FbAlarmOutput_t output;
    input->process_value = pv;
    input->elapsed_ms = ms;
    assert(FbAlarm_Execute(fb, config, input, &output) == FB_ALARM_STATUS_OK);
    assert(output.status == FB_ALARM_STATUS_OK);
    return output;
}

static void TestBasicModes(void)
{
    const struct { FbAlarmMode_t mode; int32_t pv; bool expected; } cases[] = {
        {FB_ALARM_MODE_RELATIVE_HIGH_LOW, 1101, true},
        {FB_ALARM_MODE_RELATIVE_HIGH_LOW, 1100, false},
        {FB_ALARM_MODE_RELATIVE_HIGH_LOW, 980, false},
        {FB_ALARM_MODE_RELATIVE_HIGH_LOW, 979, true},
        {FB_ALARM_MODE_RELATIVE_HIGH, 1101, true},
        {FB_ALARM_MODE_RELATIVE_HIGH, 1100, false},
        {FB_ALARM_MODE_RELATIVE_HIGH, 979, false},
        {FB_ALARM_MODE_RELATIVE_LOW, 979, true},
        {FB_ALARM_MODE_RELATIVE_LOW, 980, false},
        {FB_ALARM_MODE_RELATIVE_LOW, 1101, false},
        {FB_ALARM_MODE_ABSOLUTE_HIGH_LOW, 101, true},
        {FB_ALARM_MODE_ABSOLUTE_HIGH_LOW, 100, false},
        {FB_ALARM_MODE_ABSOLUTE_HIGH_LOW, 20, false},
        {FB_ALARM_MODE_ABSOLUTE_HIGH_LOW, 19, true},
        {FB_ALARM_MODE_ABSOLUTE_HIGH, 101, true},
        {FB_ALARM_MODE_ABSOLUTE_HIGH, 100, false},
        {FB_ALARM_MODE_ABSOLUTE_HIGH, 19, false},
        {FB_ALARM_MODE_ABSOLUTE_LOW, 19, true},
        {FB_ALARM_MODE_ABSOLUTE_LOW, 20, false},
        {FB_ALARM_MODE_ABSOLUTE_LOW, 101, false}
    };
    size_t i;
    for (i = 0U; i < sizeof(cases) / sizeof(cases[0]); ++i)
    {
        FbAlarm_t fb = {0};
        FbAlarmConfig_t config = Config(cases[i].mode);
        FbAlarmInput_t input = Input();
        FbAlarmInput_t original_input;
        FbAlarmConfig_t original_config;
        FbAlarmOutput_t output;
        input.process_value = cases[i].pv;
        (void)memcpy(&original_input, &input, sizeof(input));
        (void)memcpy(&original_config, &config, sizeof(config));
        assert(FbAlarm_Execute(&fb, &config, &input, &output) == FB_ALARM_STATUS_OK);
        assert(output.condition_active == cases[i].expected);
        assert(output.qualified_active == cases[i].expected);
        assert(output.alarm_active == cases[i].expected);
        assert(memcmp(&original_input, &input, sizeof(input)) == 0);
        assert(memcmp(&original_config, &config, sizeof(config)) == 0);
    }
}

static void TestHysteresis(void)
{
    FbAlarm_t fb = {0};
    FbAlarmConfig_t config = Config(FB_ALARM_MODE_HYSTERESIS_HIGH);
    FbAlarmInput_t input = Input();
    assert(!Step(&fb, &config, &input, 1100, 20U).alarm_active);
    assert(Step(&fb, &config, &input, 1101, 20U).alarm_active);
    assert(Step(&fb, &config, &input, 1050, 20U).alarm_active);
    assert(Step(&fb, &config, &input, 1020, 20U).alarm_active);
    assert(!Step(&fb, &config, &input, 1019, 20U).alarm_active);
    config.mode = FB_ALARM_MODE_HYSTERESIS_LOW;
    assert(!Step(&fb, &config, &input, 900, 20U).alarm_active);
    assert(Step(&fb, &config, &input, 899, 20U).alarm_active);
    assert(Step(&fb, &config, &input, 950, 20U).alarm_active);
    assert(Step(&fb, &config, &input, 980, 20U).alarm_active);
    assert(!Step(&fb, &config, &input, 981, 20U).alarm_active);
}

static void TestStandby(void)
{
    int offset;
    for (offset = -1; offset <= 1; offset += 2)
    {
        FbAlarm_t fb = {0};
        FbAlarmConfig_t config = Config(FB_ALARM_MODE_RELATIVE_HIGH);
        FbAlarmInput_t input = Input();
        FbAlarmOutput_t output;
        config.options = FB_ALARM_OPTION_STANDBY;
        output = Step(&fb, &config, &input, 1200, 0U);
        assert(!output.standby_ready && !output.alarm_active);
        output = Step(&fb, &config, &input, 1000 + offset * (config.standby_band + 1), 20U);
        assert(!output.standby_ready);
        output = Step(&fb, &config, &input, 1000 + offset * config.standby_band, 20U);
        assert(output.standby_ready && !output.alarm_active);
        assert(Step(&fb, &config, &input, 1200, 20U).alarm_active);
        input.enable = false;
        assert(FbAlarm_Execute(&fb, &config, &input, &output) == FB_ALARM_STATUS_DISABLED);
        input.enable = true;
        output = Step(&fb, &config, &input, 1200, 20U);
        assert(!output.standby_ready && !output.alarm_active);
    }
}

static void TestDelayAndIsolation(void)
{
    FbAlarm_t a = {0}, b = {0};
    FbAlarmConfig_t config = Config(FB_ALARM_MODE_ABSOLUTE_HIGH);
    FbAlarmInput_t ia = Input(), ib = Input();
    FbAlarmOutput_t output;
    config.delay_ms = 100U;
    output = Step(&a, &config, &ia, 120, UINT32_MAX);
    assert(output.condition_active && !output.qualified_active && output.delay_elapsed_ms == 0U);
    (void)Step(&b, &config, &ib, 120, 0U);
    output = Step(&a, &config, &ia, 120, 99U);
    assert(!output.alarm_active && output.delay_active && output.delay_elapsed_ms == 99U);
    output = Step(&a, &config, &ia, 120, 1U);
    assert(output.alarm_active && !output.delay_active && output.delay_elapsed_ms == 100U);
    assert(b.delay_elapsed_ms == 0U);
    assert(Step(&a, &config, &ia, 120, UINT32_MAX).alarm_active);
    output = Step(&a, &config, &ia, 100, 20U);
    assert(!output.alarm_active && !output.delay_active); /* no OFF delay */
    (void)Step(&a, &config, &ia, 120, 20U);
    (void)Step(&a, &config, &ia, 120, 90U);
    output = Step(&a, &config, &ia, 99, UINT32_MAX);
    assert(!output.alarm_active && output.delay_elapsed_ms == 0U);
    (void)Step(&a, &config, &ia, 120, 20U);
    assert(!Step(&a, &config, &ia, 120, 0U).alarm_active);
    (void)Step(&a, &config, &ia, 120, 35U);
    assert(Step(&a, &config, &ia, 120, 70U).alarm_active);
    config.delay_ms = FB_ALARM_DELAY_MAX_MS;
    (void)Step(&a, &config, &ia, 120, UINT32_MAX);
    assert(!Step(&a, &config, &ia, 120, 99999U).alarm_active);
    assert(Step(&a, &config, &ia, 120, UINT32_MAX).alarm_active);
}

static void TestManualReset(void)
{
    FbAlarm_t fb = {0};
    FbAlarmConfig_t config = Config(FB_ALARM_MODE_ABSOLUTE_HIGH);
    FbAlarmInput_t input = Input();
    FbAlarmOutput_t output;
    config.reset_mode = FB_ALARM_RESET_MANUAL;
    config.options = FB_ALARM_OPTION_PEAK;
    config.delay_ms = 100U;
    assert(!Step(&fb, &config, &input, 120, 0U).alarm_latched);
    input.reset = true;
    input.elapsed_ms = 99U;
    assert(FbAlarm_Execute(&fb, &config, &input, &output) == FB_ALARM_STATUS_RESET_REJECTED);
    assert(output.delay_elapsed_ms == 99U && !output.alarm_active);
    input.reset = false;
    assert(Step(&fb, &config, &input, 120, 1U).alarm_latched);
    input.reset = true;
    assert(FbAlarm_Execute(&fb, &config, &input, &output) == FB_ALARM_STATUS_RESET_REJECTED);
    assert(output.alarm_active && output.alarm_latched && output.peak_valid);
    output = Step(&fb, &config, &input, 90, 20U);
    assert(output.alarm_active && output.alarm_latched && !output.qualified_active);
    input.reset = false;
    (void)Step(&fb, &config, &input, 90, 20U);
    input.reset = true;
    assert(FbAlarm_Execute(&fb, &config, &input, &output) == FB_ALARM_STATUS_RESET);
    assert(!output.alarm_active && !output.alarm_latched && output.peak_valid);
    (void)Step(&fb, &config, &input, 120, 0U);
    assert(Step(&fb, &config, &input, 120, 100U).alarm_latched);
    input.reset = false;
    (void)Step(&fb, &config, &input, 90, 0U);
    input.reset = true;
    input.clear_peak = true;
    assert(FbAlarm_Execute(&fb, &config, &input, &output) == FB_ALARM_STATUS_RESET);
    assert(!output.alarm_latched && !output.peak_valid);
    config.reset_mode = FB_ALARM_RESET_AUTO;
    config.delay_ms = 0U;
    input.reset = false;
    input.clear_peak = false;
    assert(Step(&fb, &config, &input, 120, 0U).alarm_active);
    output = Step(&fb, &config, &input, 90, 0U);
    assert(!output.alarm_active && !output.alarm_latched);
}

static void TestPeakWindow(void)
{
    FbAlarm_t fb = {0};
    FbAlarmConfig_t config = Config(FB_ALARM_MODE_ABSOLUTE_HIGH);
    FbAlarmInput_t input = Input();
    FbAlarmOutput_t output;
    config.options = FB_ALARM_OPTION_PEAK;
    config.reset_mode = FB_ALARM_RESET_MANUAL;
    config.delay_ms = 20U;
    assert(!Step(&fb, &config, &input, 200, 0U).peak_valid);
    output = Step(&fb, &config, &input, 120, 20U);
    assert(output.peak_valid && output.peak_high == 120 && output.peak_low == 120);
    output = Step(&fb, &config, &input, 130, 20U);
    assert(output.peak_high == 130 && output.peak_low == 120);
    output = Step(&fb, &config, &input, -500, 20U);
    assert(output.alarm_active && !output.qualified_active && output.peak_low == 120);
    (void)Step(&fb, &config, &input, 140, 20U);
    output = Step(&fb, &config, &input, 140, 20U);
    assert(output.peak_high == 140 && output.peak_low == 120);
    input.clear_peak = true;
    output = Step(&fb, &config, &input, 125, 20U);
    assert(output.peak_high == 125 && output.peak_low == 125);
    output = Step(&fb, &config, &input, 135, 20U);
    assert(output.peak_high == 135 && output.peak_low == 125); /* held clear does not erase */
    input.enable = false;
    assert(FbAlarm_Execute(&fb, &config, &input, &output) == FB_ALARM_STATUS_DISABLED);
    assert(!output.alarm_active && output.peak_valid && output.peak_high == 135);
    input.enable = true;
    input.clear_peak = false;
    output = Step(&fb, &config, &input, 90, 20U);
    assert(output.peak_high == 135);
    input.clear_peak = true;
    output = Step(&fb, &config, &input, 90, 20U);
    assert(!output.peak_valid && output.peak_high == 0 && output.peak_low == 0);
    config.options |= FB_ALARM_OPTION_INVERT;
    output = Step(&fb, &config, &input, 90, 20U);
    assert(output.alarm_active && !output.peak_valid); /* inverted normal output is not an alarm sample */
}

static void TestDisableInvalidAndModeZero(void)
{
    FbAlarm_t fb = {0};
    FbAlarmConfig_t config = Config(FB_ALARM_MODE_ABSOLUTE_HIGH);
    FbAlarmInput_t input = Input();
    FbAlarmOutput_t output;
    config.options = FB_ALARM_OPTION_INVERT;
    assert(!Step(&fb, &config, &input, 120, 0U).alarm_active);
    input.enable = false;
    assert(FbAlarm_Execute(&fb, NULL, &input, &output) == FB_ALARM_STATUS_DISABLED);
    assert(!output.alarm_active && !output.alarm_latched);
    input.enable = true;
    input.valid = false;
    assert(FbAlarm_Execute(&fb, &config, &input, &output) == FB_ALARM_STATUS_INVALID_INPUT);
    assert(!output.alarm_active);
    input.valid = true;
    config.mode = FB_ALARM_MODE_DISABLED;
    config.options = FB_ALARM_OPTION_MASK;
    assert(FbAlarm_Execute(&fb, &config, &input, &output) == FB_ALARM_STATUS_DISABLED);
    assert(!output.alarm_active && !output.peak_valid);
    config.mode = FB_ALARM_MODE_ABSOLUTE_HIGH;
    config.options = 0U;
    config.delay_ms = 100U;
    (void)Step(&fb, &config, &input, 120, 0U);
    (void)Step(&fb, &config, &input, 120, 80U);
    input.valid = false;
    assert(FbAlarm_Execute(&fb, &config, &input, &output) == FB_ALARM_STATUS_INVALID_INPUT);
    input.valid = true;
    output = Step(&fb, &config, &input, 120, UINT32_MAX);
    assert(!output.alarm_active && output.delay_elapsed_ms == 0U);
}

static void TestConfigValidationAndRecovery(void)
{
    FbAlarm_t fb = {0};
    FbAlarmConfig_t config = Config(FB_ALARM_MODE_HYSTERESIS_HIGH);
    FbAlarmInput_t input = Input();
    FbAlarmOutput_t output;
    config.alarm_low = 101;
    assert(FbAlarm_Execute(&fb, &config, &input, &output) == FB_ALARM_STATUS_INVALID_CONFIGURATION);
    assert(!output.alarm_active);
    config.mode = FB_ALARM_MODE_ABSOLUTE_HIGH_LOW;
    assert(FbAlarm_Execute(&fb, &config, &input, &output) == FB_ALARM_STATUS_INVALID_CONFIGURATION);
    config = Config(FB_ALARM_MODE_RELATIVE_HIGH_LOW);
    config.alarm_high = -21;
    assert(FbAlarm_Execute(&fb, &config, &input, &output) == FB_ALARM_STATUS_INVALID_CONFIGURATION);
    config = Config(FB_ALARM_MODE_RELATIVE_HIGH);
    config.alarm_high = -100;
    assert(Step(&fb, &config, &input, 901, 0U).alarm_active);
    config.alarm_high = FB_ALARM_LIMIT_MIN;
    assert(Step(&fb, &config, &input, -999, 0U).alarm_active);
    config.alarm_high = FB_ALARM_LIMIT_MIN - 1;
    assert(FbAlarm_Execute(&fb, &config, &input, &output) == FB_ALARM_STATUS_INVALID_CONFIGURATION);
    config.alarm_high = FB_ALARM_LIMIT_MAX + 1;
    assert(FbAlarm_Execute(&fb, &config, &input, &output) == FB_ALARM_STATUS_INVALID_CONFIGURATION);
    config = Config(FB_ALARM_MODE_ABSOLUTE_HIGH);
    config.delay_ms = FB_ALARM_DELAY_MAX_MS + 1U;
    assert(FbAlarm_Execute(&fb, &config, &input, &output) == FB_ALARM_STATUS_INVALID_CONFIGURATION);
    config.delay_ms = 0U;
    config.options = 0x0004U; /* removed HOLD bit */
    assert(FbAlarm_Execute(&fb, &config, &input, &output) == FB_ALARM_STATUS_INVALID_CONFIGURATION);
    config.options = 0U;
    config.reset_mode = (FbAlarmResetMode_t)99;
    assert(FbAlarm_Execute(&fb, &config, &input, &output) == FB_ALARM_STATUS_INVALID_CONFIGURATION);
    config.reset_mode = FB_ALARM_RESET_AUTO;
    config.mode = (FbAlarmMode_t)99;
    assert(FbAlarm_Execute(&fb, &config, &input, &output) == FB_ALARM_STATUS_INVALID_CONFIGURATION);
    config.mode = FB_ALARM_MODE_ABSOLUTE_HIGH;
    assert(FbAlarm_Execute(&fb, NULL, &input, &output) == FB_ALARM_STATUS_INVALID_CONFIGURATION);
    assert(FbAlarm_Execute(NULL, &config, &input, &output) == FB_ALARM_STATUS_INVALID_ARGUMENT);
    assert(FbAlarm_Execute(&fb, &config, NULL, &output) == FB_ALARM_STATUS_INVALID_ARGUMENT);
    assert(FbAlarm_Execute(&fb, &config, &input, NULL) == FB_ALARM_STATUS_INVALID_ARGUMENT);
    FbAlarm_Initialize(NULL);
    assert(Step(&fb, &config, &input, 120, 0U).alarm_active);
}

static void TestReconfigurationAndWideArithmetic(void)
{
    FbAlarm_t fb;
    FbAlarmConfig_t config = Config(FB_ALARM_MODE_RELATIVE_HIGH);
    FbAlarmInput_t input = Input();
    FbAlarmOutput_t output;
    FbAlarm_Initialize(&fb);
    input.setpoint = INT32_MAX;
    assert(!Step(&fb, &config, &input, INT32_MAX, 0U).alarm_active);
    config.mode = FB_ALARM_MODE_RELATIVE_LOW;
    input.setpoint = INT32_MIN;
    assert(!Step(&fb, &config, &input, INT32_MIN, 0U).alarm_active);
    config = Config(FB_ALARM_MODE_ABSOLUTE_HIGH);
    config.delay_ms = 100U;
    (void)Step(&fb, &config, &input, 120, 20U);
    (void)Step(&fb, &config, &input, 120, 99U);
    config.delay_ms = 50U;
    output = Step(&fb, &config, &input, 120, UINT32_MAX);
    assert(!output.alarm_active && output.delay_elapsed_ms == 0U);
    assert(Step(&fb, &config, &input, 120, 50U).alarm_active);
    config.options = FB_ALARM_OPTION_PEAK;
    assert(!Step(&fb, &config, &input, 120, 50U).peak_valid);
    assert(Step(&fb, &config, &input, 120, 50U).peak_valid);
    config.alarm_high = 200;
    output = Step(&fb, &config, &input, 120, 50U);
    assert(!output.alarm_active && !output.peak_valid);
}

static void TestCombinedPipeline(void)
{
    FbAlarm_t fb = {0};
    FbAlarmConfig_t config = Config(FB_ALARM_MODE_HYSTERESIS_HIGH);
    FbAlarmInput_t input = Input();
    FbAlarmOutput_t output;
    config.options = FB_ALARM_OPTION_MASK;
    config.reset_mode = FB_ALARM_RESET_MANUAL;
    config.delay_ms = 100U;
    output = Step(&fb, &config, &input, 1200, UINT32_MAX);
    assert(!output.condition_active && !output.standby_ready && output.alarm_active);
    output = Step(&fb, &config, &input, 1000, 20U);
    assert(output.standby_ready && output.alarm_active);
    output = Step(&fb, &config, &input, 1101, 20U);
    assert(output.condition_active && !output.qualified_active && output.alarm_active);
    output = Step(&fb, &config, &input, 1050, 100U);
    assert(output.qualified_active && output.alarm_latched && !output.alarm_active);
    assert(output.peak_valid && output.peak_high == 1050 && output.peak_low == 1050);
    output = Step(&fb, &config, &input, 1000, 20U);
    assert(!output.condition_active && output.alarm_latched && !output.alarm_active);
    assert(output.peak_low == 1050);
    input.reset = true;
    assert(FbAlarm_Execute(&fb, &config, &input, &output) == FB_ALARM_STATUS_RESET);
    assert(output.alarm_active && !output.alarm_latched && output.peak_valid);
    input.reset = false;
    output = Step(&fb, &config, &input, 1200, 20U);
    assert(output.standby_ready && output.condition_active && output.peak_high == 1050);
    config.options = FB_ALARM_OPTION_PEAK;
    config.delay_ms = 0U;
    output = Step(&fb, &config, &input, 1200, 20U);
    assert(output.peak_valid && output.peak_high == 1200); /* new config clears history */
    input.clear_peak = true;
    input.enable = false;
    assert(FbAlarm_Execute(&fb, &config, &input, &output) == FB_ALARM_STATUS_DISABLED);
    assert(!output.peak_valid); /* clear is observed during disable */
}

static void TestMilliDegreeScale(void)
{
    FbAlarm_t fb = {0};
    FbAlarmConfig_t config = Config(FB_ALARM_MODE_ABSOLUTE_HIGH);
    FbAlarmInput_t input = Input();
    FbAlarmOutput_t output;
    assert(FB_ALARM_UNITS_PER_DEGREE_C == 1000);
    config.alarm_high = 25123; /* 25.123 C */
    config.alarm_low = -5123;
    config.options = FB_ALARM_OPTION_PEAK;
    assert(!Step(&fb, &config, &input, 25123, 0U).alarm_active);
    output = Step(&fb, &config, &input, 25124, 0U);
    assert(output.alarm_active && output.peak_high == 25124);
    assert(!Step(&fb, &config, &input, 25122, 0U).alarm_active);
    config.mode = FB_ALARM_MODE_ABSOLUTE_LOW;
    assert(!Step(&fb, &config, &input, -5123, 0U).alarm_active);
    output = Step(&fb, &config, &input, -5124, 0U);
    assert(output.alarm_active && output.peak_low == -5124);
    config.mode = FB_ALARM_MODE_RELATIVE_HIGH;
    config.alarm_high = 123;
    input.setpoint = 25000;
    assert(!Step(&fb, &config, &input, 25123, 0U).alarm_active);
    assert(Step(&fb, &config, &input, 25124, 0U).alarm_active);
    config.mode = FB_ALARM_MODE_HYSTERESIS_HIGH;
    config.alarm_low = 122;
    assert(Step(&fb, &config, &input, 25124, 0U).alarm_active);
    assert(Step(&fb, &config, &input, 25122, 0U).alarm_active);
    assert(!Step(&fb, &config, &input, 25121, 0U).alarm_active);
    config.mode = FB_ALARM_MODE_ABSOLUTE_HIGH;
    config.alarm_low = FB_ALARM_LIMIT_MIN;
    config.alarm_high = FB_ALARM_LIMIT_MAX;
    assert(!Step(&fb, &config, &input, 3000000, 0U).alarm_active);
    assert(Step(&fb, &config, &input, 3000001, 0U).alarm_active);
    config.mode = FB_ALARM_MODE_ABSOLUTE_LOW;
    assert(!Step(&fb, &config, &input, -200000, 0U).alarm_active);
    assert(Step(&fb, &config, &input, -200001, 0U).alarm_active);
}

static void TestConfigurableStandbyBand(void)
{
    FbAlarm_t fb = {0}, other = {0};
    FbAlarmConfig_t config = Config(FB_ALARM_MODE_RELATIVE_HIGH);
    FbAlarmInput_t input = Input();
    FbAlarmOutput_t output;
    FbAlarm_DefaultConfig(NULL);
    assert(config.standby_band == 100);
    config.options = FB_ALARM_OPTION_STANDBY | FB_ALARM_OPTION_PEAK;
    config.reset_mode = FB_ALARM_RESET_MANUAL;
    config.alarm_high = 1000;
    config.standby_band = 500;
    assert(!Step(&fb, &config, &input, 1501, 0U).standby_ready);
    assert(Step(&fb, &config, &input, 1500, 0U).standby_ready);
    assert(!Step(&other, &config, &input, 499, 0U).standby_ready);
    assert(Step(&other, &config, &input, 500, 0U).standby_ready);
    output = Step(&fb, &config, &input, 3000, 0U);
    assert(output.alarm_latched && output.peak_valid);
    config.standby_band = 0;
    output = Step(&fb, &config, &input, 1001, 0U);
    assert(!output.standby_ready && !output.alarm_latched && !output.peak_valid);
    assert(!Step(&fb, &config, &input, 999, 0U).standby_ready);
    assert(Step(&fb, &config, &input, 1000, 0U).standby_ready);
    assert(other.standby_ready && other.config.standby_band == 500);
    config.standby_band = -1;
    assert(FbAlarm_Execute(&fb, &config, &input, &output) == FB_ALARM_STATUS_INVALID_CONFIGURATION);
    assert(!output.alarm_active);
    config.standby_band = FB_ALARM_STANDBY_BAND_MAX + 1;
    assert(FbAlarm_Execute(&fb, &config, &input, &output) == FB_ALARM_STATUS_INVALID_CONFIGURATION);
    config.options = 0U; /* unused band is ignored */
    assert(Step(&fb, &config, &input, 3000, 0U).alarm_active);
    config.options = FB_ALARM_OPTION_STANDBY;
    config.standby_band = FB_ALARM_STANDBY_BAND_MAX;
    input.setpoint = INT32_MAX;
    assert(Step(&fb, &config, &input, INT32_MAX, 0U).standby_ready);
    input.setpoint = INT32_MIN;
    FbAlarm_Initialize(&fb);
    assert(Step(&fb, &config, &input, INT32_MIN, 0U).standby_ready);
}

static void TestResetHysteresisAndIndependentPeak(void)
{
    FbAlarm_t fb = {0};
    FbAlarmConfig_t config = Config(FB_ALARM_MODE_HYSTERESIS_HIGH);
    FbAlarmInput_t input = Input();
    FbAlarmOutput_t output;
    config.reset_mode = FB_ALARM_RESET_MANUAL;
    config.options = FB_ALARM_OPTION_PEAK;
    assert(Step(&fb, &config, &input, 1101, 0U).alarm_latched);
    (void)Step(&fb, &config, &input, 1050, 0U);
    input.reset = true;
    input.clear_peak = true;
    assert(FbAlarm_Execute(&fb, &config, &input, &output) == FB_ALARM_STATUS_RESET_REJECTED);
    /* Inside hysteresis: condition remains active. Clear captures current PV. */
    assert(output.alarm_latched && output.alarm_active && output.peak_valid);
    assert(output.peak_high == 1050 && output.peak_low == 1050);
    input.reset = false;
    input.clear_peak = false;
    (void)Step(&fb, &config, &input, 1019, 0U);
    input.clear_peak = true;
    output = Step(&fb, &config, &input, 1019, 0U);
    assert(output.alarm_latched && output.alarm_active && !output.peak_valid);
    input.reset = true;
    assert(FbAlarm_Execute(&fb, &config, &input, &output) == FB_ALARM_STATUS_RESET);
    assert(!output.alarm_active && !output.alarm_latched && !output.peak_valid);
    input.reset = false;
    assert(Step(&fb, &config, &input, 1101, 0U).alarm_latched);
    config.reset_mode = FB_ALARM_RESET_AUTO;
    output = Step(&fb, &config, &input, 1000, 0U);
    assert(!output.alarm_latched && !output.alarm_active && !output.peak_valid);
}

int main(void)
{
    TestBasicModes();
    TestHysteresis();
    TestStandby();
    TestDelayAndIsolation();
    TestManualReset();
    TestPeakWindow();
    TestDisableInvalidAndModeZero();
    TestConfigValidationAndRecovery();
    TestReconfigurationAndWideArithmetic();
    TestCombinedPipeline();
    TestMilliDegreeScale();
    TestConfigurableStandbyBand();
    TestResetHysteresisAndIndependentPeak();
    puts("PASS: FbAlarm 13 groups including AUTO/MANUAL reset and independent peak clear");
    return 0;
}
