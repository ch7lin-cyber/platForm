#include "FbLeadLag.h"
#include <float.h>
#include <math.h>
#include <stddef.h>
#if defined(__FAST_MATH__) || (defined(__FINITE_MATH_ONLY__) && __FINITE_MATH_ONLY__ > 0)
#error "FbLeadLag requires strict finite checks and floating-point arithmetic"
#endif
#if FLT_RADIX != 2 || FLT_MANT_DIG != 24 || FLT_MAX_EXP != 128 || \
    FLT_MIN_EXP != -125 || DBL_MANT_DIG < 53 || DBL_MAX_EXP < 1024 || DBL_MIN_EXP > -1021
#error "FbLeadLag requires binary32 float and at least binary64 double"
#endif
static bool ValidConfig(const FbLeadLagConfig *c)
{
    return isfinite(c->lead_seconds) && c->lead_seconds >= 0.0F &&
           isfinite(c->lag_seconds) && c->lag_seconds > 0.0F;
}
static bool ValidState(const FbLeadLagState *s)
{
    return s->initialized && isfinite(s->lag_value) &&
           fabs(s->lag_value) <= (double)FLT_MAX && isfinite(s->value) &&
           isfinite(s->active_lead_seconds) && s->active_lead_seconds >= 0.0F &&
           isfinite(s->active_lag_seconds) && s->active_lag_seconds > 0.0F;
}
FbLeadLagStatus FbLeadLag_Init(const FbLeadLagConfig *config,
    FbLeadLagState *state, float initial_value)
{
    if (config == NULL || state == NULL) return FB_LEAD_LAG_INVALID_ARGUMENT;
    if (!ValidConfig(config)) return FB_LEAD_LAG_INVALID_CONFIGURATION;
    if (!isfinite(initial_value)) return FB_LEAD_LAG_INVALID_INPUT;
    state->lag_value = initial_value;
    state->value = initial_value;
    state->active_lead_seconds = config->lead_seconds;
    state->active_lag_seconds = config->lag_seconds;
    state->initialized = true;
    return FB_LEAD_LAG_OK;
}
FbLeadLagStatus FbLeadLag_Reset(const FbLeadLagConfig *config,
    FbLeadLagState *state, float initial_value)
{
    return FbLeadLag_Init(config, state, initial_value);
}
FbLeadLagStatus FbLeadLag_Execute(const FbLeadLagConfig *config,
    const FbLeadLagInput *input, FbLeadLagState *state, FbLeadLagOutput *output)
{
    double lag, lead, dt, denominator, alpha, beta, delta, q, y, lower, upper;
    if (output == NULL) return FB_LEAD_LAG_INVALID_ARGUMENT;
    output->value = 0.0F;
    output->valid = false;
    output->status = FB_LEAD_LAG_INVALID_ARGUMENT;
    if (config == NULL || input == NULL || state == NULL) return output->status;
    output->status = FB_LEAD_LAG_INVALID_STATE;
    if (!ValidState(state)) return output->status;
    output->value = state->value;
    output->status = FB_LEAD_LAG_INVALID_CONFIGURATION;
    if (!ValidConfig(config) || config->lead_seconds != state->active_lead_seconds ||
        config->lag_seconds != state->active_lag_seconds) return output->status;
    output->status = FB_LEAD_LAG_NO_SAMPLE;
    if (!input->sample_valid) return output->status;
    output->status = FB_LEAD_LAG_INVALID_INPUT;
    if (!isfinite(input->value) || !isfinite(input->dt_seconds) ||
        input->dt_seconds <= 0.0F) return output->status;
    lag = config->lag_seconds;
    lead = config->lead_seconds;
    dt = input->dt_seconds;
    denominator = lag + dt;
    alpha = dt / denominator;
    beta = lag / denominator;
    delta = (double)input->value - state->lag_value;
    /* Preserve tiny updates near q, and tiny decaying tails when alpha rounds
     * to one. beta is computed independently, never as 1-alpha. */
    q = alpha <= 0.5 ? state->lag_value + alpha * delta :
                      beta * state->lag_value + alpha * (double)input->value;
    /* Roundoff must not move a convex combination outside its endpoints. */
    lower = state->lag_value < (double)input->value ? state->lag_value : (double)input->value;
    upper = state->lag_value > (double)input->value ? state->lag_value : (double)input->value;
    if (q < lower) q = lower;
    if (q > upper) q = upper;
    if (lead == lag) y = input->value;
    else if (lead == 0.0) y = q;
    else if (lead < lag) {
        double weight = (dt + lead) / denominator;
        double remainder = (lag - lead) / denominator;
        y = weight <= 0.5 ? state->lag_value + weight * delta :
            remainder * state->lag_value + weight * (double)input->value;
        if (y < lower) y = lower;
        if (y > upper) y = upper;
    } else {
        y = (double)input->value + ((lead - lag) / denominator) * delta;
    }
    output->status = FB_LEAD_LAG_NUMERIC_ERROR;
    if (!isfinite(q) || !isfinite(y) || fabs(y) > (double)FLT_MAX)
        return output->status;
    state->lag_value = q;
    state->value = (float)y;
    output->value = state->value;
    output->valid = true;
    output->status = FB_LEAD_LAG_OK;
    return output->status;
}
