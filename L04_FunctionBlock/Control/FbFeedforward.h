#ifndef FB_FEEDFORWARD_H
#define FB_FEEDFORWARD_H
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum { FB_FF_STEP=0, FB_FF_EXPONENTIAL } FbFeedforwardShape;
typedef struct {
    uint32_t duration_ms;
    int32_t target; /* 0.1%, -1500..1500 */
    FbFeedforwardShape shape;
    uint32_t tau_ms; /* explicit exponential constant, not duration */
} FbFeedforwardStage;
typedef struct {
    uint32_t delay_ms;
    FbFeedforwardStage stage1, stage2;
    uint32_t decay_ms; /* linear return to zero */
    uint32_t force_ms; /* 0=event level, otherwise hard deadline from trigger */
} FbFeedforwardConfig;
typedef struct {
    bool trigger, trigger_valid, enable, reset;
    uint32_t elapsed_ms;
} FbFeedforwardInput;
typedef enum { FB_FF_IDLE=0, FB_FF_DELAY, FB_FF_STAGE1,
    FB_FF_STAGE2, FB_FF_DECAY, FB_FF_DONE } FbFeedforwardPhase;
typedef enum { FB_FF_OK=0, FB_FF_DISABLED, FB_FF_RESET,
    FB_FF_INVALID_ARGUMENT, FB_FF_INVALID_CONFIG,
    FB_FF_INVALID_INPUT } FbFeedforwardStatus;
typedef struct {
    int32_t value;
    uint64_t elapsed_ms;
    bool valid, active;
    bool started, completed, aborted; /* one-call pulses */
    FbFeedforwardPhase phase;
    FbFeedforwardStatus status;
} FbFeedforwardOutput;
typedef struct {
    FbFeedforwardConfig config;
    uint64_t elapsed_ms;
    bool configured, previous_trigger;
    FbFeedforwardPhase phase;
} FbFeedforward;
/* Zero-init/Init required. No allocation, hardware, PID or global state.
 * Config changes cancel and require a new trigger edge. Zero-duration stages
 * are skipped without applying targets. Reset is level-sensitive, consumes
 * trigger level to prevent automatic restart. Args must not overlap. */
void FbFeedforward_Init(FbFeedforward *instance);
void FbFeedforward_DefaultConfig(FbFeedforwardConfig *config);
bool FbFeedforward_ValidateConfig(const FbFeedforwardConfig *config);
FbFeedforwardStatus FbFeedforward_Execute(FbFeedforward *instance,
    const FbFeedforwardConfig *config, const FbFeedforwardInput *input,
    FbFeedforwardOutput *output);
#ifdef __cplusplus
}
#endif
#endif
