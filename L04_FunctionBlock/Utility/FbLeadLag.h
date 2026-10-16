#ifndef FB_LEAD_LAG_H
#define FB_LEAD_LAG_H
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FB_LEAD_LAG_OK = 0,
    FB_LEAD_LAG_NO_SAMPLE,
    FB_LEAD_LAG_INVALID_ARGUMENT,
    FB_LEAD_LAG_INVALID_CONFIGURATION,
    FB_LEAD_LAG_INVALID_INPUT,
    FB_LEAD_LAG_INVALID_STATE,
    FB_LEAD_LAG_NUMERIC_ERROR
} FbLeadLagStatus;
typedef struct {
    float lead_seconds; /* finite >= 0 */
    float lag_seconds;  /* finite > 0 */
} FbLeadLagConfig;
typedef struct {
    float value;      /* finite, caller units U */
    float dt_seconds; /* finite > 0; accepted sample interval, seconds */
    bool sample_valid;
} FbLeadLagInput;
typedef struct {
    double lag_value; /* retained lowpass state, NOT the rounded output */
    float value;
    float active_lead_seconds;
    float active_lag_seconds;
    bool initialized;
} FbLeadLagState;
typedef struct {
    float value;
    bool valid;
    FbLeadLagStatus status;
} FbLeadLagOutput;

/* H(s)=(1+lead*s)/(1+lag*s), backward Euler, variable positive dt allowed.
 * q'=q+dt/(lag+dt)*(u-q); y=q'+lead/(lag+dt)*(u-q).
 * Equal taus are exact identity; zero lead is lowpass. Real lead overshoot is
 * intentional: this is NOT a safety limiter. Initial/reset value seeds q and y
 * at the same equilibrium, so constant input starts without an artificial kick.
 * Reset starts a new epoch with the supplied config, including unchanged config.
 * Init/Reset failures leave state unchanged. State must first be zero-initialized.
 */
FbLeadLagStatus FbLeadLag_Init(const FbLeadLagConfig *config,
    FbLeadLagState *state, float initial_value);
FbLeadLagStatus FbLeadLag_Reset(const FbLeadLagConfig *config,
    FbLeadLagState *state, float initial_value);
/* Config changes require Reset. Null pointers and state are checked first,
 * then config, then sample_valid. NO_SAMPLE ignores value/dt and holds all state,
 * without elapsed-time catch-up; caller supplies next accepted interval.
 * All errors hold state; output.valid=false with finite last-value diagnostic
 * (zero for invalid state), never a usable control output. Overflow is rejected,
 * not saturated. Distinct properly typed storage, no concurrent mutation.
 * Float I/O, double q preserves sub-float increments; increments below double
 * resolution may still round away. Requires IEEE-like binary32 float and at
 * least binary64 double, round-to-nearest, gradual underflow, no fast-math,
 * finite-math-only or FTZ/DAZ. Compiler guards cannot detect runtime FTZ/rounding.
 * No heap, hardware, clock, global state or L03/L05 wiring.
 */
FbLeadLagStatus FbLeadLag_Execute(const FbLeadLagConfig *config,
    const FbLeadLagInput *input, FbLeadLagState *state, FbLeadLagOutput *output);
#ifdef __cplusplus
}
#endif
#endif
