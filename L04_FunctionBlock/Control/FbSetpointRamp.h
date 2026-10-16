#ifndef FB_SETPOINT_RAMP_H
#define FB_SETPOINT_RAMP_H
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FB_SETPOINT_RAMP_OK = 0,
    FB_SETPOINT_RAMP_DISABLED,
    FB_SETPOINT_RAMP_INVALID_ARGUMENT,
    FB_SETPOINT_RAMP_INVALID_CONFIGURATION,
    FB_SETPOINT_RAMP_INVALID_INPUT,
    FB_SETPOINT_RAMP_INVALID_STATE
} FbSetpointRampStatus;

typedef struct {
    float rise_rate; /* finite, >= 0, caller-defined U/s; zero holds */
    float fall_rate; /* finite, >= 0, caller-defined U/s; zero holds */
} FbSetpointRampConfig;
typedef struct {
    float target; /* finite, caller-defined U */
    float dt_seconds; /* finite, >= 0; zero holds; no arbitrary upper bound */
    bool enable;
} FbSetpointRampInput;
typedef struct {
    float value;
    double precise_value; /* retained sub-float increments; do not modify fields */
    bool initialized;
} FbSetpointRampState;
typedef struct {
    float value;
    bool reached;
    bool valid;
    FbSetpointRampStatus status;
} FbSetpointRampOutput;

/* State must be zero-initialized before first use. No allocation. A failed
 * Init/Reset leaves state unchanged. Reset is a separate operation: no time
 * advances until the next Execute. Initial value and target share units U. */
FbSetpointRampStatus FbSetpointRamp_Init(FbSetpointRampState *state,
                                        float initial_value);
FbSetpointRampStatus FbSetpointRamp_Reset(FbSetpointRampState *state,
                                         float initial_value);
/* Errors leave state unchanged; output is invalid and carries the finite
 * initialized stored value (otherwise zero), for diagnostics only. Never use
 * invalid output as a control value. Disabled execution also holds,
 * ignores numeric input/config fields, and reports DISABLED. Null arguments
 * and invalid state are checked before enable. Non-null arguments must point
 * to distinct, correctly typed storage. No overshoot; rates apply internally
 * per cycle. precise_value retains sub-float increments; emitted float values
 * may jump by rate*dt plus up to one ULP. reached uses the precise value.
 * Increments below double resolution may still round away. */
FbSetpointRampStatus FbSetpointRamp_Execute(const FbSetpointRampConfig *config,
    const FbSetpointRampInput *input, FbSetpointRampState *state,
    FbSetpointRampOutput *output);
#ifdef __cplusplus
}
#endif
#endif
