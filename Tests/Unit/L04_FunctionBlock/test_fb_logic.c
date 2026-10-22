#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "FbLogic.h"

static FbLogicConfig_t Config(uint32_t on_ms, uint32_t off_ms)
{
    FbLogicConfig_t config = {0};
    FbLogic_DefaultConfig(&config);
    config.operator_type = FB_LOGIC_OPERATOR_OR;
    config.on_delay_ms = on_ms;
    config.off_delay_ms = off_ms;
    return config;
}

static FbLogicInput_t Input(void)
{
    FbLogicInput_t input = {0};
    input.enable = true;
    return input;
}

static FbLogicOutput_t Scan(FbLogic_t *fb, const FbLogicConfig_t *config,
                            FbLogicInput_t *input, uint16_t target, uint32_t ms)
{
    FbLogicOutput_t output;
    input->inputs[0] = target;
    input->elapsed_ms = ms;
    assert(FbLogic_Execute(fb, config, input, &output) == FB_LOGIC_STATUS_OK);
    assert(output.raw_value == target);
    return output;
}

static void TestTruthTables(void)
{
    /* Truth-table bits for input masks 0000..1111 (A is bit 0). */
    const uint16_t tables[] = {0x0000U, 0x8000U, 0xFFFEU, 0x6996U};
    unsigned op, mask, inversion, reverse, i;
    for (op = 0U; op < 4U; ++op)
        for (mask = 0U; mask < 16U; ++mask)
            for (inversion = 0U; inversion < 16U; ++inversion)
                for (reverse = 0U; reverse < 2U; ++reverse)
                {
                    FbLogic_t fb = {0};
                    FbLogicConfig_t config = Config(0U, 0U);
                    FbLogicInput_t input = Input();
                    FbLogicInput_t original;
                    FbLogicConfig_t original_config;
                    FbLogicOutput_t output;
                    uint16_t expected = (uint16_t)(
                        ((tables[op] >> (mask ^ inversion)) & 1U) ^ reverse);
                    if (op == 0U) expected = 0U; /* OFF overrides inversion */
                    config.operator_type = (FbLogicOperator_t)op;
                    config.output_inverted = reverse != 0U;
                    for (i = 0U; i < 4U; ++i)
                    {
                        input.inputs[i] = (uint16_t)(0xA500U | ((mask >> i) & 1U));
                        config.input_inversions[i] = (uint16_t)(0xFE00U |
                                                              ((inversion >> i) & 1U));
                    }
                    (void)memcpy(&original, &input, sizeof(input));
                    (void)memcpy(&original_config, &config, sizeof(config));
                    assert(FbLogic_Execute(&fb, &config, &input, &output) ==
                           (op == 0U ? FB_LOGIC_STATUS_DISABLED : FB_LOGIC_STATUS_OK));
                    assert(output.value == expected && output.raw_value == expected);
                    assert(!output.delay_active && output.delay_elapsed_ms == 0U);
                    assert(memcmp(&original, &input, sizeof(input)) == 0);
                    assert(memcmp(&original_config, &config, sizeof(config)) == 0);
                }
}

static void TestDelayBoundaries(void)
{
    FbLogic_t fb;
    FbLogicConfig_t config = Config(100U, 60U);
    FbLogicInput_t input = Input();
    FbLogicOutput_t output;
    FbLogic_Initialize(&fb);
    output = Scan(&fb, &config, &input, 1U, 5000U);
    assert(output.value == 0U && output.delay_active && output.delay_elapsed_ms == 0U);
    assert(output.on_delay_active && !output.off_delay_active);
    output = Scan(&fb, &config, &input, 1U, 99U);
    assert(output.value == 0U && output.delay_elapsed_ms == 99U);
    output = Scan(&fb, &config, &input, 1U, 1U);
    assert(output.value == 1U && !output.delay_active);
    assert(!output.on_delay_active && !output.off_delay_active);
    output = Scan(&fb, &config, &input, 0U, 5000U);
    assert(output.value == 1U && output.delay_active && output.delay_elapsed_ms == 0U);
    assert(output.off_delay_active && !output.on_delay_active);
    output = Scan(&fb, &config, &input, 0U, 59U);
    assert(output.value == 1U && output.delay_elapsed_ms == 59U);
    output = Scan(&fb, &config, &input, 0U, 1U);
    assert(output.value == 0U && !output.delay_active);
}

static void TestCancellationAndJitter(void)
{
    FbLogic_t fb = {0};
    FbLogicConfig_t config = Config(100U, 100U);
    FbLogicInput_t input = Input();
    FbLogicOutput_t output;
    (void)Scan(&fb, &config, &input, 1U, 20U);
    (void)Scan(&fb, &config, &input, 1U, 70U);
    output = Scan(&fb, &config, &input, 0U, UINT32_MAX);
    assert(output.value == 0U && !output.delay_active && output.delay_elapsed_ms == 0U);
    (void)Scan(&fb, &config, &input, 1U, 20U);
    output = Scan(&fb, &config, &input, 1U, 0U);
    assert(output.delay_elapsed_ms == 0U);
    (void)Scan(&fb, &config, &input, 1U, 35U);
    (void)Scan(&fb, &config, &input, 1U, 60U);
    output = Scan(&fb, &config, &input, 1U, 10U);
    assert(output.value == 1U && !output.delay_active);
    (void)Scan(&fb, &config, &input, 0U, 20U);
    (void)Scan(&fb, &config, &input, 0U, 90U);
    output = Scan(&fb, &config, &input, 1U, 100U);
    assert(output.value == 1U && !output.delay_active);
    output = Scan(&fb, &config, &input, 0U, 100U);
    assert(output.value == 1U && output.delay_elapsed_ms == 0U);
    output = Scan(&fb, &config, &input, 0U, 100U);
    assert(output.value == 0U);
}

static void TestDisableAndReset(void)
{
    FbLogic_t fb = {0};
    FbLogicConfig_t config = Config(100U, 200U);
    FbLogicInput_t input = Input();
    FbLogicOutput_t output;
    (void)Scan(&fb, &config, &input, 1U, 20U);
    (void)Scan(&fb, &config, &input, 1U, 100U);
    output = Scan(&fb, &config, &input, 0U, 20U);
    assert(output.value == 1U && output.delay_active);
    input.enable = false;
    config.output_inverted = true;
    assert(FbLogic_Execute(&fb, &config, &input, &output) == FB_LOGIC_STATUS_DISABLED);
    assert(output.value == 0U && !output.delay_active && fb.value == 0U);
    assert(FbLogic_Execute(&fb, NULL, &input, &output) == FB_LOGIC_STATUS_DISABLED);
    input.enable = true;
    config.output_inverted = false;
    output = Scan(&fb, &config, &input, 1U, UINT32_MAX);
    assert(output.value == 0U && output.delay_elapsed_ms == 0U);
    (void)Scan(&fb, &config, &input, 1U, 80U);
    input.enable = false;
    assert(FbLogic_Execute(&fb, &config, &input, &output) == FB_LOGIC_STATUS_DISABLED);
    assert(!output.delay_active && output.delay_elapsed_ms == 0U);
    input.enable = true;
    (void)Scan(&fb, &config, &input, 1U, 20U);
    (void)Scan(&fb, &config, &input, 1U, 100U);
    input.reset = true;
    assert(FbLogic_Execute(&fb, &config, &input, &output) == FB_LOGIC_STATUS_RESET);
    assert(output.value == 0U && !output.delay_active);
    output = Scan(&fb, &config, &input, 1U, 100U);
    assert(output.value == 0U && output.delay_active && output.delay_elapsed_ms == 0U);
    output = Scan(&fb, &config, &input, 1U, 100U);
    assert(output.value == 1U); /* held RESET must not repeatedly clear state */
    input.reset = false;
    (void)Scan(&fb, &config, &input, 1U, 0U);
    input.reset = true;
    assert(FbLogic_Execute(&fb, &config, &input, &output) == FB_LOGIC_STATUS_RESET);
    assert(output.value == 0U);
}

static void TestConfigChanges(void)
{
    FbLogic_t fb = {0};
    FbLogicConfig_t config = Config(100U, 100U);
    FbLogicInput_t input = Input();
    FbLogicOutput_t output;
    (void)Scan(&fb, &config, &input, 1U, 20U);
    (void)Scan(&fb, &config, &input, 1U, 90U);
    config.on_delay_ms = 50U;
    output = Scan(&fb, &config, &input, 1U, 100U);
    assert(output.value == 0U && output.delay_elapsed_ms == 0U);
    output = Scan(&fb, &config, &input, 1U, 50U);
    assert(output.value == 1U);
    config.off_delay_ms = 200U;
    output = Scan(&fb, &config, &input, 1U, 50U);
    assert(output.value == 0U && output.delay_elapsed_ms == 0U);
    config.on_delay_ms = 0U;
    output = Scan(&fb, &config, &input, 1U, 0U);
    assert(output.value == 1U);
    config.operator_type = FB_LOGIC_OPERATOR_AND;
    assert(FbLogic_Execute(&fb, &config, &input, &output) == FB_LOGIC_STATUS_OK);
    assert(output.value == 0U && !output.delay_active);
    config.input_inversions[1] = config.input_inversions[2] =
        config.input_inversions[3] = 1U;
    assert(FbLogic_Execute(&fb, &config, &input, &output) == FB_LOGIC_STATUS_OK);
    assert(output.value == 1U);
    config.output_inverted = true;
    assert(FbLogic_Execute(&fb, &config, &input, &output) == FB_LOGIC_STATUS_OK);
    assert(output.value == 0U && !output.delay_active);
}

static void TestInvalidArguments(void)
{
    FbLogic_t fb = {0};
    FbLogicConfig_t config = Config(0U, 100U);
    FbLogicInput_t input = Input();
    FbLogicOutput_t output;
    (void)Scan(&fb, &config, &input, 1U, 20U);
    config.operator_type = (FbLogicOperator_t)99;
    assert(FbLogic_Execute(&fb, &config, &input, &output) == FB_LOGIC_STATUS_INVALID_CONFIGURATION);
    assert(output.value == 0U && fb.value == 0U && !fb.delay_active);
    config = Config(100U, 0U);
    output = Scan(&fb, &config, &input, 1U, UINT32_MAX);
    assert(output.value == 0U && output.delay_elapsed_ms == 0U);
    assert(FbLogic_Execute(NULL, &config, &input, &output) == FB_LOGIC_STATUS_INVALID_ARGUMENT);
    assert(output.value == 0U);
    assert(FbLogic_Execute(&fb, NULL, &input, &output) == FB_LOGIC_STATUS_INVALID_ARGUMENT);
    assert(!fb.delay_active);
    assert(FbLogic_Execute(&fb, &config, NULL, &output) == FB_LOGIC_STATUS_INVALID_ARGUMENT);
    assert(FbLogic_Execute(&fb, &config, &input, NULL) == FB_LOGIC_STATUS_INVALID_ARGUMENT);
    FbLogic_Initialize(NULL);
}

static void TestOverflowAndIsolation(void)
{
    FbLogic_t a = {0}, b = {0};
    FbLogicConfig_t config = Config(UINT32_MAX, UINT32_MAX);
    FbLogicInput_t input_a = Input(), input_b = Input();
    FbLogicOutput_t output;
    (void)Scan(&a, &config, &input_a, 1U, UINT32_MAX);
    (void)Scan(&b, &config, &input_b, 1U, UINT32_MAX);
    output = Scan(&a, &config, &input_a, 1U, UINT32_MAX - 1U);
    assert(output.value == 0U && output.delay_elapsed_ms == UINT32_MAX - 1U);
    output = Scan(&a, &config, &input_a, 1U, UINT32_MAX);
    assert(output.value == 1U && !output.delay_active);
    assert(b.value == 0U && b.delay_elapsed_ms == 0U && b.delay_active);
    output = Scan(&b, &config, &input_b, 1U, UINT32_MAX);
    assert(output.value == 1U);
    (void)Scan(&a, &config, &input_a, 0U, UINT32_MAX);
    output = Scan(&a, &config, &input_a, 0U, UINT32_MAX);
    assert(output.value == 0U);
    assert(b.value == 1U);
}

static void TestInversionBeforeDelay(void)
{
    FbLogic_t fb = {0};
    FbLogicConfig_t config = Config(100U, 60U);
    FbLogicInput_t input = Input();
    FbLogicOutput_t output;
    config.output_inverted = true;
    assert(FbLogic_Execute(&fb, &config, &input, &output) == FB_LOGIC_STATUS_OK);
    assert(output.raw_value == 1U && output.value == 0U && output.delay_active);
    input.elapsed_ms = 100U;
    assert(FbLogic_Execute(&fb, &config, &input, &output) == FB_LOGIC_STATUS_OK);
    assert(output.value == 1U);
    input.inputs[0] = 1U;
    assert(FbLogic_Execute(&fb, &config, &input, &output) == FB_LOGIC_STATUS_OK);
    assert(output.raw_value == 0U && output.value == 1U && output.delay_elapsed_ms == 0U);
    input.elapsed_ms = 60U;
    assert(FbLogic_Execute(&fb, &config, &input, &output) == FB_LOGIC_STATUS_OK);
    assert(output.value == 0U && !output.delay_active);
}

static void TestZeroDelays(void)
{
    FbLogic_t fb = {0};
    FbLogicConfig_t config = Config(0U, 0U);
    FbLogicInput_t input = Input();
    FbLogicOutput_t output;
    output = Scan(&fb, &config, &input, 1U, 0U);
    assert(output.value == 1U && !output.delay_active);
    output = Scan(&fb, &config, &input, 0U, 0U);
    assert(output.value == 0U && !output.delay_active);
}

static void TestOffAndIgnoredBits(void)
{
    FbLogic_t fb = {0};
    FbLogicConfig_t config;
    FbLogicInput_t input = Input();
    FbLogicOutput_t output;
    FbLogic_DefaultConfig(NULL);
    FbLogic_DefaultConfig(&config);
    assert(config.operator_type == FB_LOGIC_OPERATOR_OFF);
    assert(config.on_delay_ms == 0U && config.off_delay_ms == 0U);
    config.output_inverted = true;
    config.on_delay_ms = config.off_delay_ms = UINT32_MAX;
    input.reset = true;
    assert(FbLogic_Execute(&fb, &config, &input, &output) == FB_LOGIC_STATUS_DISABLED);
    assert(output.value == 0U && output.raw_value == 0U && !output.delay_active);
    assert(!output.on_delay_active && !output.off_delay_active);
    /* RESET observed during OFF must not repeat when enabled. */
    config = Config(100U, 200U);
    output = Scan(&fb, &config, &input, 1U, UINT32_MAX);
    assert(output.value == 0U && output.delay_elapsed_ms == 0U);
    config.input_inversions[0] = 0xFFFEU;
    output = Scan(&fb, &config, &input, 1U, 99U);
    assert(output.on_delay_active && output.delay_elapsed_ms == 99U);
    assert(Scan(&fb, &config, &input, 1U, 1U).value == 1U);
    config.operator_type = FB_LOGIC_OPERATOR_OFF;
    config.output_inverted = true;
    assert(FbLogic_Execute(&fb, &config, &input, &output) == FB_LOGIC_STATUS_DISABLED);
    assert(output.value == 0U && !output.delay_active);
    config = Config(0U, 200U);
    assert(Scan(&fb, &config, &input, 1U, 0U).value == 1U);
    assert(Scan(&fb, &config, &input, 0U, 0U).off_delay_active);
    config.operator_type = FB_LOGIC_OPERATOR_OFF;
    assert(FbLogic_Execute(&fb, &config, &input, &output) == FB_LOGIC_STATUS_DISABLED);
    assert(output.value == 0U && !output.off_delay_active);
    /* Disable still overrides an invalid configuration. */
    input.enable = false;
    config.operator_type = (FbLogicOperator_t)99;
    assert(FbLogic_Execute(&fb, &config, &input, &output) == FB_LOGIC_STATUS_DISABLED);
}

int main(void)
{
    TestTruthTables();
    TestDelayBoundaries();
    TestCancellationAndJitter();
    TestDisableAndReset();
    TestConfigChanges();
    TestInvalidArguments();
    TestOverflowAndIsolation();
    TestInversionBeforeDelay();
    TestZeroDelays();
    TestOffAndIgnoredBits();
    puts("PASS: FbLogic 2048 truth-table combinations and 9 state/timing test groups");
    return 0;
}
