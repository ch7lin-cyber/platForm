#include "FbOutputLimiter.h"

#include <float.h>
#include <math.h>
#include <stddef.h>

static double clamp_value(double value, double lower, double upper)
{
    return value < lower ? lower : (value > upper ? upper : value);
}

/* Finite float operands widened before arithmetic: even FLT_MAX squared and
 * the distance between opposite float extremes fit in double. */
static double approach(double current, double target, float rate, float dt)
{
    double distance = (double)target - (double)current;
    double budget = (double)rate * (double)dt;
    double next;
    if (fabs(distance) <= budget) {
        return target;
    }
    next = (double)current + (distance > 0.0 ? budget : -budget);
    return clamp_value(next, current < target ? current : target,
                       current > target ? current : target);
}

FbOutputLimiterStatus FbOutputLimiter_Init(
    FbOutputLimiterState *state, float initial_value)
{
    if (state == NULL) {
        return FB_OUTPUT_LIMITER_INVALID_ARGUMENT;
    }
    if (!isfinite(initial_value)) {
        return FB_OUTPUT_LIMITER_INVALID_INPUT;
    }
    state->value = initial_value;
    state->precise_value = initial_value;
    state->initialized = 1;
    return FB_OUTPUT_LIMITER_OK;
}

FbOutputLimiterStatus FbOutputLimiter_Reset(
    FbOutputLimiterState *state, float initial_value)
{
    return FbOutputLimiter_Init(state, initial_value);
}

FbOutputLimiterStatus FbOutputLimiter_Execute(
    const FbOutputLimiterConfig *config,
    const FbOutputLimiterInput *input,
    FbOutputLimiterState *state,
    FbOutputLimiterOutput *output)
{
    double current;
    float target;
    double next;
    if (output != NULL) {
        output->value = state != NULL && state->initialized == 1 &&
                        isfinite(state->value) ? state->value : 0.0;
        output->valid = 0;
        output->status = FB_OUTPUT_LIMITER_INVALID_ARGUMENT;
        output->amplitude_limited = 0;
        output->rate_limited = 0;
    }
    if (config == NULL || input == NULL || state == NULL || output == NULL) {
        return FB_OUTPUT_LIMITER_INVALID_ARGUMENT;
    }
    if (!isfinite(config->lower_limit) || !isfinite(config->upper_limit) ||
        config->lower_limit > config->upper_limit ||
        !isfinite(config->rise_rate) || config->rise_rate < 0.0 ||
        !isfinite(config->fall_rate) || config->fall_rate < 0.0) {
        output->status = FB_OUTPUT_LIMITER_INVALID_CONFIGURATION;
        return output->status;
    }
    if (state->initialized != 1 || !isfinite(state->value) ||
        !isfinite(state->precise_value) ||
        state->precise_value < -FLT_MAX || state->precise_value > FLT_MAX ||
        (float)state->precise_value != state->value) {
        output->status = FB_OUTPUT_LIMITER_INVALID_STATE;
        return output->status;
    }
    if (input->enable && (!isfinite(input->requested) ||
        !isfinite(input->dt_seconds) || input->dt_seconds < 0.0)) {
        output->status = FB_OUTPUT_LIMITER_INVALID_INPUT;
        return output->status;
    }
    current = clamp_value(state->precise_value, config->lower_limit, config->upper_limit);
    output->amplitude_limited = current != state->precise_value;
    if (!input->enable) {
        state->value = (float)current;
        state->precise_value = current;
        output->value = current;
        output->status = FB_OUTPUT_LIMITER_DISABLED;
        return output->status;
    }
    target = clamp_value(input->requested, config->lower_limit, config->upper_limit);
    output->amplitude_limited |= target != input->requested;
    next = approach(current, target,
                    target > current ? config->rise_rate : config->fall_rate,
                    input->dt_seconds);
    state->value = (float)next;
    state->precise_value = next;
    output->value = next;
    output->valid = 1;
    output->status = FB_OUTPUT_LIMITER_OK;
    output->rate_limited = next != target;
    return FB_OUTPUT_LIMITER_OK;
}
