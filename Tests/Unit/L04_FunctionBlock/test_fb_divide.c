#include "FbDivide.h"

#include <assert.h>
#include <math.h>

static void ExpectStatus(FbDivideConfig_t *config,
                         FbDivideInput_t *input,
                         FbDivideOutput_t *output,
                         FbDivideStatus_t status)
{
    assert(!FbDivide_Run(config, input, output));
    assert(output->valid == false);
    assert(output->quotient == 0.0F);
    assert(output->status == status);
}

int main(void)
{
    FbDivideConfig_t config;
    FbDivideInput_t input = {12.0F, 3.0F, true};
    FbDivideOutput_t output = {99.0F, true, FB_DIVIDE_STATUS_OK};

    FbDivide_Initialize(&config);
    assert(config.denominator_epsilon == 0.0F);
    assert(FbDivide_Run(&config, &input, &output));
    assert(output.valid && output.status == FB_DIVIDE_STATUS_OK);
    assert(output.quotient == 4.0F);

    input.numerator = -12.0F;
    input.denominator = -3.0F;
    assert(FbDivide_Run(&config, &input, &output));
    assert(output.quotient == 4.0F);

    input.denominator = 0.0F;
    ExpectStatus(&config, &input, &output, FB_DIVIDE_STATUS_DIVIDE_BY_ZERO);

    config.denominator_epsilon = 0.01F;
    input.denominator = 0.01F;
    ExpectStatus(&config, &input, &output, FB_DIVIDE_STATUS_DIVIDE_BY_ZERO);
    input.denominator = -0.005F;
    ExpectStatus(&config, &input, &output, FB_DIVIDE_STATUS_DIVIDE_BY_ZERO);
    input.denominator = 0.02F;
    assert(FbDivide_Run(&config, &input, &output));
    assert(output.quotient == -600.0F);

    input.enable = false;
    ExpectStatus(&config, &input, &output, FB_DIVIDE_STATUS_DISABLED);
    input.enable = true;

    input.numerator = NAN;
    ExpectStatus(&config, &input, &output, FB_DIVIDE_STATUS_INVALID_INPUT);
    input.numerator = 1.0F;
    input.denominator = INFINITY;
    ExpectStatus(&config, &input, &output, FB_DIVIDE_STATUS_INVALID_INPUT);

    input.denominator = 1.0F;
    config.denominator_epsilon = NAN;
    ExpectStatus(&config, &input, &output,
                 FB_DIVIDE_STATUS_INVALID_CONFIGURATION);
    config.denominator_epsilon = -1.0F;
    ExpectStatus(&config, &input, &output,
                 FB_DIVIDE_STATUS_INVALID_CONFIGURATION);
    config.denominator_epsilon = INFINITY;
    ExpectStatus(&config, &input, &output,
                 FB_DIVIDE_STATUS_INVALID_CONFIGURATION);
    config.denominator_epsilon = 0.0F;

    input.numerator = 1.0e30F;
    input.denominator = 1.0e-30F;
    ExpectStatus(&config, &input, &output,
                 FB_DIVIDE_STATUS_NONFINITE_RESULT);

    FbDivide_Reset(&output);
    assert(output.quotient == 0.0F);
    assert(!output.valid && output.status == FB_DIVIDE_STATUS_DISABLED);

    assert(!FbDivide_Run(NULL, &input, &output));
    assert(output.status == FB_DIVIDE_STATUS_INVALID_ARGUMENT);
    assert(!FbDivide_Run(&config, NULL, &output));
    assert(output.status == FB_DIVIDE_STATUS_INVALID_ARGUMENT);
    assert(!FbDivide_Run(&config, &input, NULL));

    return 0;
}
