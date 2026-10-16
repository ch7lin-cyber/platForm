#ifndef FB_OUTPUT_LIMITER_H
#define FB_OUTPUT_LIMITER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FB_OUTPUT_LIMITER_OK = 0,
    FB_OUTPUT_LIMITER_INVALID_ARGUMENT,
    FB_OUTPUT_LIMITER_INVALID_INPUT,
    FB_OUTPUT_LIMITER_INVALID_CONFIGURATION,
    FB_OUTPUT_LIMITER_INVALID_STATE,
    FB_OUTPUT_LIMITER_DISABLED
} FbOutputLimiterStatus;

typedef struct {
    float lower_limit; /* U; finite, <= upper_limit */
    float upper_limit; /* U; finite */
    float rise_rate;   /* U/s; finite, >= 0; zero holds */
    float fall_rate;   /* U/s; finite, >= 0; zero holds */
} FbOutputLimiterConfig;

typedef struct {
    float requested;  /* U; finite when enabled */
    float dt_seconds; /* s; finite, >= 0 when enabled; zero holds */
    bool enable;       /* zero holds, except mandatory hard-bound correction */
} FbOutputLimiterInput;

typedef struct {
    float value; /* U; caller must not modify after initialization */
    double precise_value; /* U; retained accumulator, rounds to value */
    bool initialized;
} FbOutputLimiterState;

typedef struct {
    float value; /* U */
    FbOutputLimiterStatus status;
    bool valid; /* true only on OK; disabled and errors are invalid */
    bool amplitude_limited; /* request or previous state needed hard clamping */
    bool rate_limited; /* precise_value differs from clamped request, even if float output rounds to it */
} FbOutputLimiterOutput;

/* Init and Reset are equivalent; finite initial_value is stored without bounds.
 * Bounds are enforced at the next successful Execute, including disabled calls.
 * Failure leaves state unchanged. Zero-initialize state if testing before Init.
 * No allocation, global state, hardware access or implicit elapsed-time source.
 */
FbOutputLimiterStatus FbOutputLimiter_Init(
    FbOutputLimiterState *state, float initial_value);
FbOutputLimiterStatus FbOutputLimiter_Reset(
    FbOutputLimiterState *state, float initial_value);

/* Hard bounds always override slew limits, even after configuration changes or
 * while disabled. Enabled calls clamp old state and request before rate limiting.
 * Disabled calls ignore requested/dt_seconds, hold the clamped state and return
 * DISABLED with valid=0. No catch-up time is retained. Finite large dt saturates
 * at the clamped target, never overshoots. Retained double precise_value avoids
 * per-cycle float rounding stalls. Visible float output is quantized: its step
 * may exceed rate*dt by up to one local float ULP. Invalid enabled inputs/config/state
 * leave state unchanged; output (if non-NULL) is invalid, flags clear and value
 * is the finite initialized held value, or zero if unavailable. Such error
 * output is diagnostic only and is NOT guaranteed inside invalid/new bounds.
 * Config/state are validated before disabled handling; pointers must refer to
 * distinct valid objects. Status is returned and mirrored in Output when non-NULL.
 */
FbOutputLimiterStatus FbOutputLimiter_Execute(
    const FbOutputLimiterConfig *config,
    const FbOutputLimiterInput *input,
    FbOutputLimiterState *state,
    FbOutputLimiterOutput *output);

#ifdef __cplusplus
}
#endif
#endif /* FB_OUTPUT_LIMITER_H */
