#ifndef FB_SIGNAL_LINEARIZATION_H
#define FB_SIGNAL_LINEARIZATION_H

#include "FbLinearization.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Stateless single-signal wrappers. All values use caller-defined units.
 * Source routing, unit conversion and Pending/Apply belong to the caller.
 * Tables are borrowed immutable storage for the duration of Execute.
 * No input/config/output storage may overlap. No allocation or Reset needed. */
typedef enum {
    FB_INPUT_LINEARIZATION_BYPASS = 0,
    FB_INPUT_LINEARIZATION_VALUE,
    FB_INPUT_LINEARIZATION_DEVIATION
} FbInputLinearizationMode;

typedef enum {
    FB_OUTPUT_LINEARIZATION_OFF = 0,
    FB_OUTPUT_LINEARIZATION_INTERPOLATED,
    FB_OUTPUT_LINEARIZATION_STEPPED
} FbOutputLinearizationMode;

typedef struct {
    FbLinearizationConfig curve; /* explicit count >=2, strictly increasing X */
    bool offset_enabled;
    float offset; /* output Y units; for deviation, X and Y must match */
    bool limits_enabled;
    float limit_low;
    float limit_high;
    FbLinearizationPolicy limit_policy; /* applied after conversion + offset */
} FbSignalLinearizationParameters;

typedef struct {
    FbInputLinearizationMode mode;
    FbSignalLinearizationParameters parameters;
} FbInputLinearizationConfig;

typedef struct {
    FbOutputLinearizationMode mode;
    FbSignalLinearizationParameters parameters;
} FbOutputLinearizationConfig;

typedef struct {
    float value;
    bool valid;
    bool enable; /* disabled => zero/invalid, distinct from input BYPASS */
} FbSignalLinearizationInput;

typedef struct {
    float value;
    bool valid;
    bool bypassed;
    bool limited; /* final output limits, not curve-domain clamping */
    FbLinearizationRange range; /* input domain; UNKNOWN for bypass/failure */
    FbLinearizationStatus status;
} FbSignalLinearizationOutput;

/* BYPASS copies a finite valid input, ignores curve/offset/limits.
 * VALUE=f(x); DEVIATION=x+f(x); optional offset follows both modes.
 * OFF ignores all numeric/config fields and returns DISABLED, zero/invalid.
 * STEPPED: Xi<=x<Xi+1 => Yi, exact last node => last Y.
 * Curve bounds use CLAMP/ERROR; no extrapolation. Final limits use their own
 * CLAMP/ERROR policy. Overflow/nonfinite result => INVALID_INPUT, zero/invalid.
 * Null arguments precede enable; all failures clear output diagnostics.
 * Unit metadata is deliberately outside this mathematical API. */
FbLinearizationStatus FbInputLinearization_Execute(
    const FbInputLinearizationConfig *config,
    const FbSignalLinearizationInput *input,
    FbSignalLinearizationOutput *output);
FbLinearizationStatus FbOutputLinearization_Execute(
    const FbOutputLinearizationConfig *config,
    const FbSignalLinearizationInput *input,
    FbSignalLinearizationOutput *output);

#ifdef __cplusplus
}
#endif

#endif
