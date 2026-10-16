#include "FbLinearization.h"

#include <float.h>
#include <math.h>

#if defined(__FAST_MATH__)
#error "FbLinearization does not support unsafe fast-math reassociation"
#endif

#if FLT_RADIX != 2 || DBL_MANT_DIG < 2 * FLT_MANT_DIG || \
    DBL_MAX_EXP < 2 * FLT_MAX_EXP + 2 || \
    DBL_MIN_EXP > 2 * (FLT_MIN_EXP - FLT_MANT_DIG)
#error "FbLinearization requires double to hold exact float products and sums"
#endif

static bool ValidConfiguration(const FbLinearizationConfig *config)
{
    size_t index;

    if (config->table == NULL || config->count < 2 ||
        (config->policy != FB_LINEARIZATION_CLAMP &&
         config->policy != FB_LINEARIZATION_ERROR)) {
        return false;
    }
    for (index = 0; index < config->count; ++index) {
        if (!isfinite(config->table[index].x) ||
            !isfinite(config->table[index].y) ||
            (index > 0 &&
             !(config->table[index].x > config->table[index - 1].x))) {
            return false;
        }
    }
    return true;
}

/* Grow an exact, non-overlapping expansion by one term using TwoSum.
 * Each source product is exact in double under the numeric contract above.
 * Four calls need at most four slots; no heap or retained state is used. */
static void AddTerm(double expansion[4], size_t *count, double term)
{
    size_t index;
    size_t used = 0;

    for (index = 0; index < *count; ++index) {
        double part = expansion[index];
        double sum = term + part;
        double part_virtual = sum - term;
        double term_virtual = sum - part_virtual;
        double part_error = part - part_virtual;
        double term_error = term - term_virtual;
        double error = term_error + part_error;

        if (error != 0.0) expansion[used++] = error;
        term = sum;
    }
    if (term != 0.0 || used == 0) expansion[used++] = term;
    *count = used;
}

static float Interpolate(const FbLinearizationPoint *left,
                         const FbLinearizationPoint *right, float value)
{
    double expansion[4];
    size_t count = 0;
    size_t index;
    double numerator = 0.0;
    double lower = left->y < right->y ? left->y : right->y;
    double upper = left->y > right->y ? left->y : right->y;
    double result;

    /* y = (yL*xR - yR*xL + x*yR - x*yL) / (xR-xL).
     * Expanding the products before summing preserves a small result when
     * large terms cancel, e.g. x=y=+/-FLT_MAX evaluated at x=1. */
    AddTerm(expansion, &count, (double)left->y * (double)right->x);
    AddTerm(expansion, &count, -(double)right->y * (double)left->x);
    AddTerm(expansion, &count, (double)value * (double)right->y);
    AddTerm(expansion, &count, -(double)value * (double)left->y);
    for (index = 0; index < count; ++index) numerator += expansion[index];
    result = numerator / ((double)right->x - (double)left->x);
    /* Bound the double before conversion, including same-sign FLT_MAX y. */
    if (result < lower) result = lower;
    if (result > upper) result = upper;
    return (float)result;
}

FbLinearizationStatus FbLinearization_Execute(
    const FbLinearizationConfig *config, const FbLinearizationInput *input,
    FbLinearizationOutput *output)
{
    const FbLinearizationPoint *table;
    size_t index;

    if (output == NULL) return FB_LINEARIZATION_INVALID_ARGUMENT;
    output->value = 0.0F;
    output->range = FB_LINEARIZATION_RANGE_UNKNOWN;
    output->valid = false;
    output->status = FB_LINEARIZATION_INVALID_ARGUMENT;
    if (config == NULL || input == NULL) return output->status;
    if (!input->enable) {
        output->status = FB_LINEARIZATION_DISABLED;
        return output->status;
    }
    if (!ValidConfiguration(config)) {
        output->status = FB_LINEARIZATION_INVALID_CONFIGURATION;
        return output->status;
    }
    if (!isfinite(input->value)) {
        output->status = FB_LINEARIZATION_INVALID_INPUT;
        return output->status;
    }

    table = config->table;
    if (input->value < table[0].x ||
        input->value > table[config->count - 1].x) {
        if (config->policy == FB_LINEARIZATION_ERROR) {
            output->status = FB_LINEARIZATION_OUT_OF_RANGE;
            return output->status;
        }
        if (input->value < table[0].x) {
            output->value = table[0].y;
            output->range = FB_LINEARIZATION_RANGE_BELOW;
        } else {
            output->value = table[config->count - 1].y;
            output->range = FB_LINEARIZATION_RANGE_ABOVE;
        }
    } else {
        for (index = 0; index < config->count; ++index) {
            if (input->value == table[index].x) {
                output->value = table[index].y;
                break;
            }
            if (input->value < table[index].x) {
                output->value = Interpolate(&table[index - 1],
                                             &table[index], input->value);
                break;
            }
        }
        output->range = FB_LINEARIZATION_RANGE_WITHIN;
    }
    output->valid = true;
    output->status = FB_LINEARIZATION_OK;
    return output->status;
}
