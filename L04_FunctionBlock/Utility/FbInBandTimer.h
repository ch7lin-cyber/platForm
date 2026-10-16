#ifndef FB_IN_BAND_TIMER_H
#define FB_IN_BAND_TIMER_H
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FB_IN_BAND_TIMER_OK = 0,
    FB_IN_BAND_TIMER_DISABLED,
    FB_IN_BAND_TIMER_INVALID_ARGUMENT,
    FB_IN_BAND_TIMER_INVALID_CONFIGURATION,
    FB_IN_BAND_TIMER_INVALID_INPUT,
    FB_IN_BAND_TIMER_INVALID_STATE
} FbInBandTimerStatus;
typedef enum {
    FB_IN_BAND_TIMER_PAUSE = 0,
    FB_IN_BAND_TIMER_RESET
} FbInBandTimerOutOfBandPolicy;
typedef struct {
    float tolerance; /* finite >= 0, same units as pv/sp; inclusive band */
    float dwell_seconds; /* finite >= 0; elapsed saturates at this value */
    FbInBandTimerOutOfBandPolicy out_of_band_policy;
} FbInBandTimerConfig;
typedef struct {
    float pv;
    float sp;
    float dt_seconds; /* finite >= 0; current sample represents whole interval */
    bool enable;
    bool input_valid; /* caller measurement quality; false freezes with INVALID_INPUT */
} FbInBandTimerInput;
typedef struct {
    double elapsed_seconds;
    double compensation; /* Kahan correction: elapsed - compensation */
    bool initialized;
} FbInBandTimerState;
typedef struct {
    float elapsed_seconds; /* rounded diagnostic value; use done, not float equality */
    bool in_band;
    bool done; /* current sample in band AND dwell reached; not latched */
    bool valid;
    FbInBandTimerStatus status;
} FbInBandTimerOutput;

/* Zero-initialize state before first Execute; Init/Reset clears time and residual.
 * State fields are implementation-owned. No heap, clock, hardware or global state. */
FbInBandTimerStatus FbInBandTimer_Init(FbInBandTimerState *state);
FbInBandTimerStatus FbInBandTimer_Reset(FbInBandTimerState *state);
/* Null arguments and state checked before enable. Disabled ignores numeric
 * config/input and freezes all state. Errors also freeze state. Invalid outputs
 * have false flags and a finite elapsed diagnostic (zero if invalid state).
 * Enabled calls validate all numerics even out of band. Zero dt still evaluates
 * band/reset and caps elapsed to a lowered dwell. PAUSE retains elapsed outside
 * band; RESET clears it. Completion is non-latching. SP/config changes never
 * implicitly start a new process: call Reset explicitly. Lowering dwell caps
 * history; raising it resumes accumulation without restoring discarded time.
 * Distinct correctly typed storage required; no concurrent mutation. Requires
 * wider double (compile-time checked), round-to-nearest, no unsafe floating
 * reassociation or flush-to-zero. Finite precision, not an exact clock. */
FbInBandTimerStatus FbInBandTimer_Execute(const FbInBandTimerConfig *config,
    const FbInBandTimerInput *input, FbInBandTimerState *state,
    FbInBandTimerOutput *output);
#ifdef __cplusplus
}
#endif
#endif
