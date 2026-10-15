#include "FbWeightedSum.h"

#include <assert.h>
#include <math.h>
#include <stddef.h>

static void ExpectStatus(FbWeightedSumConfig_t *config,
                         FbWeightedSumInput_t *input,
                         FbWeightedSumOutput_t *output,
                         FbWeightedSumStatus_t status)
{
    assert(!FbWeightedSum_Run(config, input, output));
    assert(!output->valid);
    assert(output->sum == 0.0F);
    assert(output->status == status);
}

int main(void)
{
    FbWeightedSumConfig_t config;
    FbWeightedSumInput_t input = {{1.0F, 2.0F, 3.0F, 4.0F},
                                  {0.5F, 1.0F, -1.0F, 2.0F}, true};
    FbWeightedSumOutput_t output = {99.0F, true, FB_WEIGHTED_SUM_STATUS_OK};

    FbWeightedSum_Initialize(&config);
    assert(config.input_count == FB_WEIGHTED_SUM_INPUT_COUNT);
    assert(FbWeightedSum_Run(&config, &input, &output));
    assert(output.valid && output.status == FB_WEIGHTED_SUM_STATUS_OK);
    assert(output.sum == 7.5F);

    config.input_count = 1U;
    assert(FbWeightedSum_Run(&config, &input, &output));
    assert(output.sum == 0.5F);

    config.input_count = 3U;
    input.values[3] = NAN;
    assert(FbWeightedSum_Run(&config, &input, &output));
    assert(output.sum == -0.5F);

    config.input_count = 0U;
    ExpectStatus(&config, &input, &output,
                 FB_WEIGHTED_SUM_STATUS_INVALID_CONFIGURATION);
    config.input_count = FB_WEIGHTED_SUM_INPUT_COUNT + 1U;
    ExpectStatus(&config, &input, &output,
                 FB_WEIGHTED_SUM_STATUS_INVALID_CONFIGURATION);
    config.input_count = FB_WEIGHTED_SUM_INPUT_COUNT;

    input.values[0] = NAN;
    ExpectStatus(&config, &input, &output,
                 FB_WEIGHTED_SUM_STATUS_INVALID_INPUT);
    input.values[0] = 1.0F;
    input.weights[0] = INFINITY;
    ExpectStatus(&config, &input, &output,
                 FB_WEIGHTED_SUM_STATUS_INVALID_INPUT);
    input.weights[0] = 0.5F;

    input.enable = false;
    ExpectStatus(&config, &input, &output, FB_WEIGHTED_SUM_STATUS_DISABLED);
    input.enable = true;

    input.values[0] = 1.0e30F;
    input.weights[0] = 1.0e30F;
    ExpectStatus(&config, &input, &output,
                 FB_WEIGHTED_SUM_STATUS_NONFINITE_RESULT);
    input.values[0] = 2.0e38F;
    input.weights[0] = 1.0F;
    input.values[1] = 2.0e38F;
    input.weights[1] = 1.0F;
    input.values[2] = 3.0F;
    input.weights[2] = -1.0F;
    input.values[3] = 4.0F;
    ExpectStatus(&config, &input, &output,
                 FB_WEIGHTED_SUM_STATUS_NONFINITE_RESULT);

    FbWeightedSum_Reset(&output);
    assert(output.sum == 0.0F);
    assert(!output.valid &&
           output.status == FB_WEIGHTED_SUM_STATUS_DISABLED);

    assert(!FbWeightedSum_Run(NULL, &input, &output));
    assert(output.status == FB_WEIGHTED_SUM_STATUS_INVALID_ARGUMENT);
    assert(!FbWeightedSum_Run(&config, NULL, &output));
    assert(output.status == FB_WEIGHTED_SUM_STATUS_INVALID_ARGUMENT);
    assert(!FbWeightedSum_Run(&config, &input, NULL));

    return 0;
}
