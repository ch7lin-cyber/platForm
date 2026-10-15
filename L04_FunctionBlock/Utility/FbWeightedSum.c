#include "FbWeightedSum.h"

#include <math.h>
#include <stddef.h>

static void SetFailure(FbWeightedSumOutput_t *output,
                       FbWeightedSumStatus_t status)
{
    output->sum = 0.0F;
    output->valid = false;
    output->status = status;
}

void FbWeightedSum_Initialize(FbWeightedSumConfig_t *config)
{
    if (config != NULL)
    {
        config->input_count = FB_WEIGHTED_SUM_INPUT_COUNT;
    }
}

void FbWeightedSum_Reset(FbWeightedSumOutput_t *output)
{
    if (output != NULL)
    {
        SetFailure(output, FB_WEIGHTED_SUM_STATUS_DISABLED);
    }
}

bool FbWeightedSum_Run(const FbWeightedSumConfig_t *config,
                       const FbWeightedSumInput_t *input,
                       FbWeightedSumOutput_t *output)
{
    float sum = 0.0F;
    uint8_t index;

    if (output == NULL)
    {
        return false;
    }

    if ((config == NULL) || (input == NULL))
    {
        SetFailure(output, FB_WEIGHTED_SUM_STATUS_INVALID_ARGUMENT);
        return false;
    }

    if ((config->input_count == 0U) ||
        (config->input_count > FB_WEIGHTED_SUM_INPUT_COUNT))
    {
        SetFailure(output, FB_WEIGHTED_SUM_STATUS_INVALID_CONFIGURATION);
        return false;
    }

    if (!input->enable)
    {
        SetFailure(output, FB_WEIGHTED_SUM_STATUS_DISABLED);
        return false;
    }

    for (index = 0U; index < config->input_count; index++)
    {
        float term;

        if (!isfinite(input->values[index]) ||
            !isfinite(input->weights[index]))
        {
            SetFailure(output, FB_WEIGHTED_SUM_STATUS_INVALID_INPUT);
            return false;
        }

        term = input->values[index] * input->weights[index];
        if (!isfinite(term))
        {
            SetFailure(output, FB_WEIGHTED_SUM_STATUS_NONFINITE_RESULT);
            return false;
        }

        sum += term;
        if (!isfinite(sum))
        {
            SetFailure(output, FB_WEIGHTED_SUM_STATUS_NONFINITE_RESULT);
            return false;
        }
    }

    output->sum = sum;
    output->valid = true;
    output->status = FB_WEIGHTED_SUM_STATUS_OK;
    return true;
}
