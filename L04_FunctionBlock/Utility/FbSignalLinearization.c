#include "FbSignalLinearization.h"
#include <float.h>
#include <math.h>

static FbLinearizationStatus Clear(FbSignalLinearizationOutput *out,
                                   FbLinearizationStatus status)
{
    if (out != NULL) {
        *out = (FbSignalLinearizationOutput){0};
        out->status = status;
    }
    return status;
}

static bool PolicyValid(FbLinearizationPolicy policy)
{
    return policy == FB_LINEARIZATION_CLAMP || policy == FB_LINEARIZATION_ERROR;
}

static FbLinearizationStatus Calculate(
    const FbSignalLinearizationParameters *p,
    const FbSignalLinearizationInput *in,
    FbSignalLinearizationOutput *out, bool stepped, bool deviation)
{
    FbLinearizationOutput curve;
    FbLinearizationInput value = {in->value, true};
    FbLinearizationStatus status;
    double result;
    size_t i;
    if ((p->offset_enabled && !isfinite(p->offset)) ||
        (p->limits_enabled && (!isfinite(p->limit_low) ||
         !isfinite(p->limit_high) || p->limit_low > p->limit_high ||
         !PolicyValid(p->limit_policy))))
        return Clear(out, FB_LINEARIZATION_INVALID_CONFIGURATION);
    /* Reuse core validation and exact-node/domain rules for both operations. */
    status = FbLinearization_Execute(&p->curve, &value, &curve);
    if (status != FB_LINEARIZATION_OK) return Clear(out, status);
    if (stepped && curve.range == FB_LINEARIZATION_RANGE_WITHIN) {
        for (i = 1; i < p->curve.count; ++i)
            if (in->value < p->curve.table[i].x) break;
        curve.value = p->curve.table[i - 1].y;
    }
    result = (double)curve.value;
    if (deviation) result += (double)in->value;
    if (p->offset_enabled) result += (double)p->offset;
    if (p->limits_enabled) {
        if (result < p->limit_low || result > p->limit_high) {
            if (p->limit_policy == FB_LINEARIZATION_ERROR)
                return Clear(out, FB_LINEARIZATION_OUT_OF_RANGE);
            result = result < p->limit_low ? p->limit_low : p->limit_high;
            out->limited = true;
        }
    }
    if (!isfinite(result) || result > FLT_MAX || result < -FLT_MAX)
        return Clear(out, FB_LINEARIZATION_INVALID_INPUT);
    out->value = (float)result;
    out->valid = true;
    out->range = curve.range;
    out->status = FB_LINEARIZATION_OK;
    return out->status;
}

FbLinearizationStatus FbInputLinearization_Execute(
    const FbInputLinearizationConfig *config,
    const FbSignalLinearizationInput *input,
    FbSignalLinearizationOutput *output)
{
    Clear(output, FB_LINEARIZATION_INVALID_ARGUMENT);
    if (config == NULL || input == NULL || output == NULL)
        return FB_LINEARIZATION_INVALID_ARGUMENT;
    if (!input->enable) return Clear(output, FB_LINEARIZATION_DISABLED);
    if (config->mode < FB_INPUT_LINEARIZATION_BYPASS ||
        config->mode > FB_INPUT_LINEARIZATION_DEVIATION)
        return Clear(output, FB_LINEARIZATION_INVALID_CONFIGURATION);
    if (!input->valid || !isfinite(input->value))
        return Clear(output, FB_LINEARIZATION_INVALID_INPUT);
    if (config->mode == FB_INPUT_LINEARIZATION_BYPASS) {
        output->value = input->value;
        output->valid = true;
        output->bypassed = true;
        output->status = FB_LINEARIZATION_OK;
        return output->status;
    }
    return Calculate(&config->parameters, input, output, false,
                     config->mode == FB_INPUT_LINEARIZATION_DEVIATION);
}

FbLinearizationStatus FbOutputLinearization_Execute(
    const FbOutputLinearizationConfig *config,
    const FbSignalLinearizationInput *input,
    FbSignalLinearizationOutput *output)
{
    Clear(output, FB_LINEARIZATION_INVALID_ARGUMENT);
    if (config == NULL || input == NULL || output == NULL)
        return FB_LINEARIZATION_INVALID_ARGUMENT;
    if (!input->enable || config->mode == FB_OUTPUT_LINEARIZATION_OFF)
        return Clear(output, FB_LINEARIZATION_DISABLED);
    if (config->mode < FB_OUTPUT_LINEARIZATION_OFF ||
        config->mode > FB_OUTPUT_LINEARIZATION_STEPPED)
        return Clear(output, FB_LINEARIZATION_INVALID_CONFIGURATION);
    if (!input->valid || !isfinite(input->value))
        return Clear(output, FB_LINEARIZATION_INVALID_INPUT);
    return Calculate(&config->parameters, input, output,
                     config->mode == FB_OUTPUT_LINEARIZATION_STEPPED, false);
}
