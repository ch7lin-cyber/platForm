#ifndef FB_PROFILE_SEQUENCER_H
#define FB_PROFILE_SEQUENCER_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define FB_PROFILE_MAX_PATTERNS 16u
#define FB_PROFILE_MAX_STEPS 64u
#define FB_PROFILE_TRANSITION_LIMIT 64u
typedef struct { int32_t target; uint32_t duration_ms; } FbProfileStep;
typedef enum { FB_PROFILE_LINK=0, FB_PROFILE_END, FB_PROFILE_STOP } FbProfileLink;
typedef struct {
    const FbProfileStep *steps;
    size_t count;
    uint16_t repeat_count; /* additional repeats */
    FbProfileLink link;
    size_t next_pattern;
} FbProfilePattern;
typedef struct {
    const FbProfilePattern *patterns;
    size_t count, start_pattern, start_step;
    uint32_t revision; /* update on every recipe edit; immutable while active */
    uint32_t starting_rate; /* milli-C/min, zero-duration first step only */
    bool wait_enabled;
    uint32_t wait_band, wait_timeout_ms; /* timeout=0 means unlimited */
} FbProfileConfig;
typedef enum { FB_PROFILE_CMD_NONE=0, FB_PROFILE_CMD_START, FB_PROFILE_CMD_STOP,
    FB_PROFILE_CMD_HOLD, FB_PROFILE_CMD_RESUME, FB_PROFILE_CMD_ADVANCE,
    FB_PROFILE_CMD_RESET } FbProfileCommand;
typedef struct {
    int32_t initial_sv, pv;
    bool pv_valid, enable;
    uint32_t elapsed_ms;
    FbProfileCommand command; /* changed command edge; NONE rearms repeats */
} FbProfileInput;
typedef enum { FB_PROFILE_IDLE=0, FB_PROFILE_RUNNING, FB_PROFILE_WAITING,
    FB_PROFILE_HELD, FB_PROFILE_COMPLETED, FB_PROFILE_STOPPED,
    FB_PROFILE_FAULT } FbProfilePhase;
typedef enum { FB_PROFILE_OK=0, FB_PROFILE_DISABLED, FB_PROFILE_INVALID_ARGUMENT,
    FB_PROFILE_INVALID_CONFIG, FB_PROFILE_INVALID_INPUT, FB_PROFILE_WAIT_TIMEOUT,
    FB_PROFILE_TRANSITION_OVERFLOW } FbProfileStatus;
typedef struct {
    int32_t sv, target;
    size_t pattern, step;
    uint16_t repetition;
    uint32_t segment_elapsed_ms, segment_duration_ms, waiting_ms;
    bool valid, completed, stop_requested, timeout_requested;
    FbProfilePhase phase;
    FbProfileStatus status;
} FbProfileOutput;
typedef struct {
    FbProfileConfig config;
    int32_t sv, segment_start;
    size_t pattern, step;
    uint16_t repetition;
    uint32_t elapsed_ms, duration_ms, waiting_ms;
    bool configured, started;
    FbProfilePhase phase, held_phase;
    FbProfileCommand previous_command;
    FbProfileStatus fault_status;
} FbProfileSequencer;
/* Zero-init/Init required, no allocation. Recipes borrowed immutable while
 * active. No hardware/NVM. Non-overlapping valid arguments required.
 * START/HOLD/RESUME/ADVANCE discard that call's dt. Disable clears all.
 * Fault requires Reset or disable; no automatic heating restart. */
void FbProfileSequencer_Init(FbProfileSequencer *instance);
bool FbProfileSequencer_ValidateConfig(const FbProfileConfig *config);
FbProfileStatus FbProfileSequencer_Execute(FbProfileSequencer *instance,
    const FbProfileConfig *config, const FbProfileInput *input, FbProfileOutput *output);
#ifdef __cplusplus
}
#endif
#endif
