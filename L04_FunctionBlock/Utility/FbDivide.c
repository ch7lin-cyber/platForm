#include "FbDivide.h"

#include <math.h>
#include <stddef.h>

static void SetFailure(FbDivideOutput_t *output, FbDivideStatus_t status)
{
    output->quotient = 0.0F;
    output->valid = false;
    output->status = status;
}

void FbDivide_Initialize(FbDivideConfig_t *config)
{
    if (config != NULL)
    {
        config->denominator_epsilon = 0.0F;
    }
}

void FbDivide_Reset(FbDivideOutput_t *output)
{
    if (output != NULL)
    {
        SetFailure(output, FB_DIVIDE_STATUS_DISABLED);
    }
}

bool FbDivide_Run(const FbDivideConfig_t *config,
                  const FbDivideInput_t *input,
                  FbDivideOutput_t *output)
{
    float quotient;

    if (output == NULL)
    {
        return false;
    }

    if ((config == NULL) || (input == NULL))
    {
        SetFailure(output, FB_DIVIDE_STATUS_INVALID_ARGUMENT);
        return false;
    }

    if (!isfinite(config->denominator_epsilon) ||
        (config->denominator_epsilon < 0.0F))
    {
        SetFailure(output, FB_DIVIDE_STATUS_INVALID_CONFIGURATION);
        return false;
    }

    if (!input->enable)
    {
        SetFailure(output, FB_DIVIDE_STATUS_DISABLED);
        return false;
    }

    if (!isfinite(input->numerator) || !isfinite(input->denominator))
    {
        SetFailure(output, FB_DIVIDE_STATUS_INVALID_INPUT);
        return false;
    }

    if ((input->denominator >= -config->denominator_epsilon) &&
        (input->denominator <= config->denominator_epsilon))
    {
        SetFailure(output, FB_DIVIDE_STATUS_DIVIDE_BY_ZERO);
        return false;
    }

    quotient = input->numerator / input->denominator;
    if (!isfinite(quotient))
    {
        SetFailure(output, FB_DIVIDE_STATUS_NONFINITE_RESULT);
        return false;
    }

    output->quotient = quotient;
    output->valid = true;
    output->status = FB_DIVIDE_STATUS_OK;
    return true;
}
