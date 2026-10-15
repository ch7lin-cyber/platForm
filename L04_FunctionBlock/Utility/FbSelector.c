#include "FbSelector.h"

#include <math.h>
#include <stddef.h>

static void SetFailure(FbSelectorOutput_t *output,
                       FbSelectorStatus_t status)
{
    output->value = 0.0F;
    output->selected_index = 0U;
    output->valid = false;
    output->status = status;
}

void FbSelector_Initialize(FbSelectorConfig_t *config)
{
    if (config != NULL)
    {
        config->mode = FB_SELECTOR_MODE_INDEX;
        config->input_count = FB_SELECTOR_INPUT_COUNT;
        config->selected_index = 0U;
    }
}

void FbSelector_Reset(FbSelectorOutput_t *output)
{
    if (output != NULL)
    {
        SetFailure(output, FB_SELECTOR_STATUS_DISABLED);
    }
}

bool FbSelector_Run(const FbSelectorConfig_t *config,
                    const FbSelectorInput_t *input,
                    FbSelectorOutput_t *output)
{
    uint8_t index;
    uint8_t selected_index;

    if (output == NULL)
    {
        return false;
    }

    if ((config == NULL) || (input == NULL))
    {
        SetFailure(output, FB_SELECTOR_STATUS_INVALID_ARGUMENT);
        return false;
    }

    if ((config->input_count == 0U) ||
        (config->input_count > FB_SELECTOR_INPUT_COUNT) ||
        ((config->mode != FB_SELECTOR_MODE_INDEX) &&
         (config->mode != FB_SELECTOR_MODE_MINIMUM) &&
         (config->mode != FB_SELECTOR_MODE_MAXIMUM)) ||
        ((config->mode == FB_SELECTOR_MODE_INDEX) &&
         (config->selected_index >= config->input_count)))
    {
        SetFailure(output, FB_SELECTOR_STATUS_INVALID_CONFIGURATION);
        return false;
    }

    if (!input->enable)
    {
        SetFailure(output, FB_SELECTOR_STATUS_DISABLED);
        return false;
    }

    selected_index = (config->mode == FB_SELECTOR_MODE_INDEX)
                         ? config->selected_index
                         : 0U;

    if (!isfinite(input->values[selected_index]))
    {
        SetFailure(output, FB_SELECTOR_STATUS_INVALID_INPUT);
        return false;
    }

    if (config->mode != FB_SELECTOR_MODE_INDEX)
    {
        for (index = 1U; index < config->input_count; index++)
        {
            const float value = input->values[index];
            const float selected_value = input->values[selected_index];
            const bool is_better =
                (config->mode == FB_SELECTOR_MODE_MINIMUM)
                    ? (value < selected_value)
                    : (value > selected_value);

            if (!isfinite(value))
            {
                SetFailure(output, FB_SELECTOR_STATUS_INVALID_INPUT);
                return false;
            }

            if (is_better)
            {
                selected_index = index;
            }
        }
    }

    output->value = input->values[selected_index];
    output->selected_index = selected_index;
    output->valid = true;
    output->status = FB_SELECTOR_STATUS_OK;
    return true;
}
