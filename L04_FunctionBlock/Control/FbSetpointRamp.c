#include "FbSetpointRamp.h"
#include <math.h>
#include <float.h>
#include <stddef.h>

/* Every finite float difference and rate*dt product fits in double. */
static double advance(double current, float target, float rate, float dt)
{
    double distance = (double)target - (double)current;
    double travel = (double)rate * (double)dt;
    double result;
    if (travel >= fabs(distance)) return target;
    result = (double)current + (distance > 0.0 ? travel : -travel);
    return result;
}
FbSetpointRampStatus FbSetpointRamp_Init(FbSetpointRampState *state,
                                        float initial_value)
{
    if (state == NULL) return FB_SETPOINT_RAMP_INVALID_ARGUMENT;
    if (!isfinite(initial_value)) return FB_SETPOINT_RAMP_INVALID_INPUT;
    state->value = initial_value;
    state->precise_value = initial_value;
    state->initialized = true;
    return FB_SETPOINT_RAMP_OK;
}
FbSetpointRampStatus FbSetpointRamp_Reset(FbSetpointRampState *state,
                                         float initial_value)
{
    return FbSetpointRamp_Init(state, initial_value);
}
FbSetpointRampStatus FbSetpointRamp_Execute(const FbSetpointRampConfig *config,
    const FbSetpointRampInput *input, FbSetpointRampState *state,
    FbSetpointRampOutput *output)
{
    float rate;
    if (output == NULL) return FB_SETPOINT_RAMP_INVALID_ARGUMENT;
    output->value = state != NULL && state->initialized && isfinite(state->value)
                        ? state->value : 0.0;
    output->valid = false;
    output->reached = false;
    output->status = FB_SETPOINT_RAMP_INVALID_ARGUMENT;
    if (config == NULL || input == NULL || state == NULL) return output->status;
    if (!state->initialized || !isfinite(state->value) ||
        !isfinite(state->precise_value) ||
        fabs(state->precise_value) > FLT_MAX ||
        (float)state->precise_value != state->value) {
        output->status = FB_SETPOINT_RAMP_INVALID_STATE;
    } else if (!input->enable) {
        output->status = FB_SETPOINT_RAMP_DISABLED;
    } else if (!isfinite(config->rise_rate) || config->rise_rate < 0.0 ||
               !isfinite(config->fall_rate) || config->fall_rate < 0.0) {
        output->status = FB_SETPOINT_RAMP_INVALID_CONFIGURATION;
    } else if (!isfinite(input->target) || !isfinite(input->dt_seconds) ||
               input->dt_seconds < 0.0) {
        output->status = FB_SETPOINT_RAMP_INVALID_INPUT;
    } else {
        rate = input->target > state->precise_value ? config->rise_rate : config->fall_rate;
        state->precise_value = advance(state->precise_value, input->target, rate, input->dt_seconds);
        state->value = (float)state->precise_value;
        output->value = state->value;
        output->reached = state->precise_value == input->target;
        output->valid = true;
        output->status = FB_SETPOINT_RAMP_OK;
    }
    return output->status;
}
