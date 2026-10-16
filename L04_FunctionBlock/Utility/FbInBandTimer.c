#include "FbInBandTimer.h"
#include <float.h>
#include <math.h>
#include <stddef.h>

#if defined(__FAST_MATH__)
#error "FbInBandTimer requires finite checks and non-reassociated accumulation"
#endif
#if DBL_MAX_EXP <= FLT_MAX_EXP || DBL_MANT_DIG <= FLT_MANT_DIG
#error "FbInBandTimer requires double wider than float"
#endif

static bool state_valid(const FbInBandTimerState *state)
{
    return state != NULL && state->initialized &&
        isfinite(state->elapsed_seconds) && state->elapsed_seconds >= 0.0 &&
        state->elapsed_seconds <= FLT_MAX && isfinite(state->compensation) &&
        fabs(state->compensation) <= state->elapsed_seconds * DBL_EPSILON;
}
static void clear_time(FbInBandTimerState *state)
{
    state->elapsed_seconds = 0.0;
    state->compensation = 0.0;
}
static void cap_time(FbInBandTimerState *state, double dwell)
{
    if (state->elapsed_seconds > dwell ||
        (state->elapsed_seconds == dwell && state->compensation <= 0.0)) {
        state->elapsed_seconds = dwell;
        state->compensation = 0.0;
    }
}
FbInBandTimerStatus FbInBandTimer_Init(FbInBandTimerState *state)
{
    if (state == NULL) return FB_IN_BAND_TIMER_INVALID_ARGUMENT;
    clear_time(state);
    state->initialized = true;
    return FB_IN_BAND_TIMER_OK;
}
FbInBandTimerStatus FbInBandTimer_Reset(FbInBandTimerState *state)
{
    return FbInBandTimer_Init(state);
}
FbInBandTimerStatus FbInBandTimer_Execute(const FbInBandTimerConfig *config,
    const FbInBandTimerInput *input, FbInBandTimerState *state,
    FbInBandTimerOutput *output)
{
    bool good_state = state_valid(state);
    if (output == NULL) return FB_IN_BAND_TIMER_INVALID_ARGUMENT;
    output->elapsed_seconds = good_state ? (float)state->elapsed_seconds : 0.0f;
    output->in_band = false;
    output->done = false;
    output->valid = false;
    output->status = FB_IN_BAND_TIMER_INVALID_ARGUMENT;
    if (config == NULL || input == NULL || state == NULL) return output->status;
    if (!good_state) {
        output->status = FB_IN_BAND_TIMER_INVALID_STATE;
    } else if (!input->enable) {
        output->status = FB_IN_BAND_TIMER_DISABLED;
    } else if (!isfinite(config->tolerance) || config->tolerance < 0.0f ||
        !isfinite(config->dwell_seconds) || config->dwell_seconds < 0.0f ||
        (config->out_of_band_policy != FB_IN_BAND_TIMER_PAUSE &&
         config->out_of_band_policy != FB_IN_BAND_TIMER_RESET)) {
        output->status = FB_IN_BAND_TIMER_INVALID_CONFIGURATION;
    } else if (!input->input_valid || !isfinite(input->pv) || !isfinite(input->sp) ||
        !isfinite(input->dt_seconds) || input->dt_seconds < 0.0f) {
        output->status = FB_IN_BAND_TIMER_INVALID_INPUT;
    } else {
        output->in_band = fabs((double)input->pv - (double)input->sp) <= config->tolerance;
        cap_time(state, config->dwell_seconds);
        if (!output->in_band && config->out_of_band_policy == FB_IN_BAND_TIMER_RESET) {
            clear_time(state);
        } else if (output->in_band && input->dt_seconds > 0.0f) {
            /* Float operands sum safely in double; compensated accumulation
             * retains increments even when they are below double's ULP. */
            double increment = (double)input->dt_seconds - state->compensation;
            double sum = state->elapsed_seconds + increment;
            state->compensation = (sum - state->elapsed_seconds) - increment;
            state->elapsed_seconds = sum;
            cap_time(state, config->dwell_seconds);
        }
        output->elapsed_seconds = (float)state->elapsed_seconds;
        output->done = output->in_band && state->elapsed_seconds == config->dwell_seconds &&
                       state->compensation == 0.0;
        output->valid = true;
        output->status = FB_IN_BAND_TIMER_OK;
    }
    return output->status;
}
