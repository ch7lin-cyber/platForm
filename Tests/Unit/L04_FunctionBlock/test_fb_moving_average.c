#include "FbMovingAverage.h"

#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>

static void expect_near(double actual, double expected)
{
    assert(fabs(actual - expected) < 1e-9);
}

int main(void)
{
    double buffer[4] = {0.0};
    FbMovingAverageState state;
    FbMovingAverageConfig config = {3U};
    FbMovingAverageInput input = {0.0, 1, 0, 0};
    FbMovingAverageOutput output;

    assert(FbMovingAverage_Init(&state, buffer, 4U) == FB_MOVING_AVERAGE_OK);
    assert(FbMovingAverage_Execute(&config, &input, &state, &output) == FB_MOVING_AVERAGE_OK);
    assert(!output.valid && output.sample_count == 0U);

    input.sample_valid = 1;
    input.sample = 3.0;
    assert(FbMovingAverage_Execute(&config, &input, &state, &output) == FB_MOVING_AVERAGE_OK);
    assert(output.valid && output.sample_count == 1U);
    expect_near(output.average, 3.0);

    input.sample = 6.0;
    assert(FbMovingAverage_Execute(&config, &input, &state, &output) == FB_MOVING_AVERAGE_OK);
    assert(output.sample_count == 2U);
    expect_near(output.average, 4.5);

    input.sample_valid = 0;
    input.sample = NAN;
    assert(FbMovingAverage_Execute(&config, &input, &state, &output) == FB_MOVING_AVERAGE_OK);
    assert(output.sample_count == 2U);
    expect_near(output.average, 4.5);

    input.sample_valid = 1;
    input.sample = 9.0;
    assert(FbMovingAverage_Execute(&config, &input, &state, &output) == FB_MOVING_AVERAGE_OK);
    assert(output.sample_count == 3U);
    expect_near(output.average, 6.0);

    input.sample = 12.0;
    assert(FbMovingAverage_Execute(&config, &input, &state, &output) == FB_MOVING_AVERAGE_OK);
    expect_near(output.average, 9.0); /* oldest (3) was replaced */

    input.reset = 1;
    input.sample = 15.0;
    assert(FbMovingAverage_Execute(&config, &input, &state, &output) == FB_MOVING_AVERAGE_OK);
    assert(output.sample_count == 1U);
    expect_near(output.average, 15.0);
    input.reset = 0;

    config.window_size = 2U; /* changing size starts a new window */
    input.sample = -2.0;
    assert(FbMovingAverage_Execute(&config, &input, &state, &output) == FB_MOVING_AVERAGE_OK);
    assert(output.sample_count == 1U);
    expect_near(output.average, -2.0);
    input.sample = 4.0;
    assert(FbMovingAverage_Execute(&config, &input, &state, &output) == FB_MOVING_AVERAGE_OK);
    expect_near(output.average, 1.0);

    input.sample = INFINITY;
    assert(FbMovingAverage_Execute(&config, &input, &state, &output) == FB_MOVING_AVERAGE_INVALID_INPUT);
    assert(output.sample_count == 2U);
    expect_near(output.average, 1.0);

    config.window_size = 0U;
    assert(FbMovingAverage_Execute(&config, &input, &state, &output) == FB_MOVING_AVERAGE_INVALID_ARGUMENT);

    config.window_size = 5U;
    assert(FbMovingAverage_Execute(&config, &input, &state, &output) == FB_MOVING_AVERAGE_INVALID_ARGUMENT);

    config.window_size = 3U;
    input.enable = 0;
    input.sample_valid = 1;
    assert(FbMovingAverage_Execute(&config, &input, &state, &output) == FB_MOVING_AVERAGE_DISABLED);
    assert(!output.valid && output.sample_count == 0U);
    input.enable = 1;
    input.sample = 10.0;
    assert(FbMovingAverage_Execute(&config, &input, &state, &output) == FB_MOVING_AVERAGE_OK);
    assert(output.sample_count == 1U);
    expect_near(output.average, 10.0);

    assert(FbMovingAverage_Init(NULL, buffer, 4U) == FB_MOVING_AVERAGE_INVALID_ARGUMENT);
    assert(FbMovingAverage_Init(&state, NULL, 4U) == FB_MOVING_AVERAGE_INVALID_ARGUMENT);
    assert(FbMovingAverage_Init(&state, buffer, 0U) == FB_MOVING_AVERAGE_INVALID_ARGUMENT);

    puts("FbMovingAverage host tests passed");
    return 0;
}
