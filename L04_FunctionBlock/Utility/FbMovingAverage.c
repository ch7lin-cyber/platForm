#include "FbMovingAverage.h"

#include <float.h>
#include <string.h>

/* Ordered comparisons reject NaN and both infinities without requiring
 * a C library classification symbol (Redlib's isfinite uses __isfinited). */
static int sample_is_finite(double value)
{
    return value >= -DBL_MAX && value <= DBL_MAX;
}

static void clear_window(FbMovingAverageState *state)
{
    state->next_index = 0U;
    state->sample_count = 0U;
    state->sum = 0.0L;
    state->active_window_size = 0U;
    if (state->buffer != NULL && state->buffer_capacity > 0U) {
        memset(state->buffer, 0, state->buffer_capacity * sizeof(state->buffer[0]));
    }
}

FbMovingAverageStatus FbMovingAverage_Init(
    FbMovingAverageState *state,
    double *buffer,
    size_t buffer_capacity)
{
    if (state == NULL || buffer == NULL || buffer_capacity == 0U) {
        return FB_MOVING_AVERAGE_INVALID_ARGUMENT;
    }

    state->buffer = buffer;
    state->buffer_capacity = buffer_capacity;
    state->initialized = 1;
    clear_window(state);
    return FB_MOVING_AVERAGE_OK;
}

FbMovingAverageStatus FbMovingAverage_Execute(
    const FbMovingAverageConfig *config,
    const FbMovingAverageInput *input,
    FbMovingAverageState *state,
    FbMovingAverageOutput *output)
{
    if (config == NULL || input == NULL || state == NULL || output == NULL ||
        !state->initialized || state->buffer == NULL ||
        state->buffer_capacity == 0U || config->window_size == 0U ||
        config->window_size > state->buffer_capacity) {
        return FB_MOVING_AVERAGE_INVALID_ARGUMENT;
    }
    if (input->sample_valid && !sample_is_finite(input->sample)) {
        return FB_MOVING_AVERAGE_INVALID_INPUT;
    }

    if (input->reset || state->active_window_size != config->window_size) {
        clear_window(state);
        state->active_window_size = config->window_size;
    }

    if (input->sample_valid) {
        if (state->sample_count == config->window_size) {
            state->sum -= (long double)state->buffer[state->next_index];
        } else {
            ++state->sample_count;
        }

        state->buffer[state->next_index] = input->sample;
        state->sum += (long double)input->sample;
        state->next_index = (state->next_index + 1U) % config->window_size;
    }

    output->sample_count = state->sample_count;
    output->valid = state->sample_count > 0U;
    output->average = output->valid ? (double)(state->sum / (long double)state->sample_count) : 0.0;
    if (!sample_is_finite(output->average)) {
        return FB_MOVING_AVERAGE_INVALID_INPUT;
    }
    return FB_MOVING_AVERAGE_OK;
}
