#include "FbDeadTime.h"
#include <math.h>
#if defined(__FAST_MATH__) || (defined(__FINITE_MATH_ONLY__) && __FINITE_MATH_ONLY__ > 0)
#error "FbDeadTime requires reliable finite checks"
#endif

static bool storage_valid(size_t n, float *buffer, size_t capacity)
{
    return n <= (size_t)-1 / sizeof(float) &&
        capacity <= (size_t)-1 / sizeof(float) && capacity >= n &&
        (buffer != NULL || capacity == 0);
}
static bool state_valid(const FbDeadTimeState *s)
{
    if (s == NULL || !s->initialized ||
        !storage_valid(s->delay_samples, s->buffer, s->capacity) ||
        !isfinite(s->initial_value) || !isfinite(s->last_value) ||
        s->filled > s->delay_samples) return false;
    if (s->delay_samples == 0) return s->write_index == 0 && s->filled == 0;
    return s->write_index < s->delay_samples &&
        (s->filled == s->delay_samples ||
         (s->write_index == s->filled && s->last_value == s->initial_value));
}
FbDeadTimeStatus FbDeadTime_Init(const FbDeadTimeConfig *config,
    FbDeadTimeState *state, float *buffer, size_t capacity, float initial_value)
{
    if (config == NULL || state == NULL) return FB_DEAD_TIME_INVALID_ARGUMENT;
    if (!storage_valid(config->delay_samples, buffer, capacity))
        return FB_DEAD_TIME_INVALID_CONFIGURATION;
    if (!isfinite(initial_value)) return FB_DEAD_TIME_INVALID_INPUT;
    state->buffer = buffer;
    state->capacity = capacity;
    state->delay_samples = config->delay_samples;
    state->write_index = 0;
    state->filled = 0;
    state->initial_value = initial_value;
    state->last_value = initial_value;
    state->initialized = true;
    return FB_DEAD_TIME_OK;
}
FbDeadTimeStatus FbDeadTime_Reset(const FbDeadTimeConfig *config,
    FbDeadTimeState *state, float *buffer, size_t capacity, float initial_value)
{
    return FbDeadTime_Init(config, state, buffer, capacity, initial_value);
}
FbDeadTimeStatus FbDeadTime_Execute(const FbDeadTimeConfig *config,
    const FbDeadTimeInput *input, FbDeadTimeState *state, FbDeadTimeOutput *output)
{
    bool good_state;
    float value;
    bool ready;
    if (output == NULL) return FB_DEAD_TIME_INVALID_ARGUMENT;
    good_state = state_valid(state);
    output->value = good_state ? state->last_value : 0.0f;
    output->valid = false;
    output->ready = false;
    output->status = FB_DEAD_TIME_INVALID_ARGUMENT;
    if (config == NULL || input == NULL || state == NULL) return output->status;
    if (!good_state) output->status = FB_DEAD_TIME_INVALID_STATE;
    else if (config->delay_samples != state->delay_samples)
        output->status = FB_DEAD_TIME_INVALID_CONFIGURATION;
    else if (!input->sample_valid) output->status = FB_DEAD_TIME_NO_SAMPLE;
    else if (!isfinite(input->value)) output->status = FB_DEAD_TIME_INVALID_INPUT;
    else {
        ready = state->filled == state->delay_samples;
        value = state->delay_samples == 0 ? input->value :
            (ready ? state->buffer[state->write_index] : state->initial_value);
        if (!isfinite(value)) {
            output->status = FB_DEAD_TIME_INVALID_STATE;
            return output->status;
        }
        /* All validation, including the delayed read, precedes every mutation. */
        if (state->delay_samples != 0) {
            state->buffer[state->write_index] = input->value;
            if (!ready) ++state->filled;
            ++state->write_index;
            if (state->write_index == state->delay_samples) state->write_index = 0;
        }
        state->last_value = value;
        output->value = value;
        output->valid = true;
        output->ready = ready;
        output->status = FB_DEAD_TIME_OK;
    }
    return output->status;
}
