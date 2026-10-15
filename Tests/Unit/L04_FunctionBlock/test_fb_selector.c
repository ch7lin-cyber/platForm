#include "FbSelector.h"

#include <assert.h>
#include <math.h>
#include <stddef.h>

static void ExpectStatus(FbSelectorConfig_t *config,
                         FbSelectorInput_t *input,
                         FbSelectorOutput_t *output,
                         FbSelectorStatus_t status)
{
    assert(!FbSelector_Run(config, input, output));
    assert(!output->valid);
    assert(output->value == 0.0F);
    assert(output->selected_index == 0U);
    assert(output->status == status);
}

int main(void)
{
    FbSelectorConfig_t config;
    FbSelectorInput_t input = {{5.0F, -3.0F, -3.0F, 8.0F}, true};
    FbSelectorOutput_t output = {99.0F, 3U, true, FB_SELECTOR_STATUS_OK};

    FbSelector_Initialize(&config);
    assert(config.mode == FB_SELECTOR_MODE_INDEX);
    assert(config.input_count == FB_SELECTOR_INPUT_COUNT);
    assert(config.selected_index == 0U);

    config.selected_index = 2U;
    assert(FbSelector_Run(&config, &input, &output));
    assert(output.valid && output.status == FB_SELECTOR_STATUS_OK);
    assert(output.value == -3.0F && output.selected_index == 2U);

    config.mode = FB_SELECTOR_MODE_MINIMUM;
    assert(FbSelector_Run(&config, &input, &output));
    assert(output.value == -3.0F && output.selected_index == 1U);

    config.mode = FB_SELECTOR_MODE_MAXIMUM;
    assert(FbSelector_Run(&config, &input, &output));
    assert(output.value == 8.0F && output.selected_index == 3U);

    config.input_count = 3U;
    input.values[3] = NAN;
    assert(FbSelector_Run(&config, &input, &output));
    assert(output.value == 5.0F && output.selected_index == 0U);

    config.mode = FB_SELECTOR_MODE_INDEX;
    config.selected_index = 3U;
    ExpectStatus(&config, &input, &output,
                 FB_SELECTOR_STATUS_INVALID_CONFIGURATION);
    config.selected_index = 0U;
    config.input_count = 0U;
    ExpectStatus(&config, &input, &output,
                 FB_SELECTOR_STATUS_INVALID_CONFIGURATION);
    config.input_count = FB_SELECTOR_INPUT_COUNT + 1U;
    ExpectStatus(&config, &input, &output,
                 FB_SELECTOR_STATUS_INVALID_CONFIGURATION);
    config.input_count = FB_SELECTOR_INPUT_COUNT;
    config.mode = (FbSelectorMode_t)99;
    ExpectStatus(&config, &input, &output,
                 FB_SELECTOR_STATUS_INVALID_CONFIGURATION);
    config.mode = (FbSelectorMode_t)-1;
    ExpectStatus(&config, &input, &output,
                 FB_SELECTOR_STATUS_INVALID_CONFIGURATION);

    config.mode = FB_SELECTOR_MODE_INDEX;
    config.selected_index = 2U;
    input.values[2] = INFINITY;
    ExpectStatus(&config, &input, &output,
                 FB_SELECTOR_STATUS_INVALID_INPUT);
    input.values[2] = -3.0F;
    config.mode = FB_SELECTOR_MODE_MINIMUM;
    input.values[1] = NAN;
    ExpectStatus(&config, &input, &output,
                 FB_SELECTOR_STATUS_INVALID_INPUT);
    input.values[1] = -3.0F;

    input.enable = false;
    ExpectStatus(&config, &input, &output, FB_SELECTOR_STATUS_DISABLED);
    input.enable = true;

    FbSelector_Reset(&output);
    assert(output.value == 0.0F);
    assert(!output.valid && output.status == FB_SELECTOR_STATUS_DISABLED);

    assert(!FbSelector_Run(NULL, &input, &output));
    assert(output.status == FB_SELECTOR_STATUS_INVALID_ARGUMENT);
    assert(!FbSelector_Run(&config, NULL, &output));
    assert(output.status == FB_SELECTOR_STATUS_INVALID_ARGUMENT);
    assert(!FbSelector_Run(&config, &input, NULL));

    return 0;
}
