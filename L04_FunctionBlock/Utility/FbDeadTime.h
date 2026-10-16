#ifndef FB_DEAD_TIME_H
#define FB_DEAD_TIME_H
#include <stdbool.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FB_DEAD_TIME_OK = 0,
    FB_DEAD_TIME_NO_SAMPLE,
    FB_DEAD_TIME_INVALID_ARGUMENT,
    FB_DEAD_TIME_INVALID_CONFIGURATION,
    FB_DEAD_TIME_INVALID_INPUT,
    FB_DEAD_TIME_INVALID_STATE
} FbDeadTimeStatus;
typedef struct {
    size_t delay_samples; /* N accepted samples, not elapsed time */
} FbDeadTimeConfig;
typedef struct {
    float value;
    bool sample_valid; /* false: hold everything, ignore numeric value */
} FbDeadTimeInput;
typedef struct {
    float *buffer;
    size_t capacity;
    size_t delay_samples;
    size_t write_index;
    size_t filled;
    float initial_value;
    float last_value;
    bool initialized;
} FbDeadTimeState;
typedef struct {
    float value;
    bool valid;
    bool ready; /* true only when this call emits an actual delayed sample */
    FbDeadTimeStatus status;
} FbDeadTimeOutput;

/* Zero-initialize State before use. State/storage belong to the caller but
 * fields and active ring contents are implementation-owned during an epoch.
 * All arguments and buffer must have distinct, nonoverlapping, correctly typed
 * storage. No concurrent access. Buffer must outlive the epoch, capacity >= N.
 * N=0 permits NULL buffer with capacity=0 (also accepts an ordinary buffer).
 * Init/Reset validate first, leave state AND buffer untouched on failure, and
 * never clear the buffer: startup fill is implicit, O(1). Initial value finite.
 * Config N is immutable until explicit Init/Reset; changing N is rejected. */
FbDeadTimeStatus FbDeadTime_Init(const FbDeadTimeConfig *config,
    FbDeadTimeState *state, float *buffer, size_t capacity, float initial_value);
FbDeadTimeStatus FbDeadTime_Reset(const FbDeadTimeConfig *config,
    FbDeadTimeState *state, float *buffer, size_t capacity, float initial_value);
/* First N accepted calls return initial_value, valid=true, ready=false.
 * Call N+1 emits first accepted sample. N=0 passes through, ready=true.
 * Priority: pointers, state, config epoch, sample_valid, numeric input, current
 * ring read. NO_SAMPLE ignores input value, does not access ring, valid/ready
 * false. All failures freeze state/buffer. Writable output holds last finite
 * diagnostic value (zero for structurally invalid state), valid/ready=false.
 * Only the current ring read is checked, not a full buffer scan. Requires
 * conforming finite checks, no fast-math or flush-to-zero. No heap/hardware. */
FbDeadTimeStatus FbDeadTime_Execute(const FbDeadTimeConfig *config,
    const FbDeadTimeInput *input, FbDeadTimeState *state, FbDeadTimeOutput *output);
#ifdef __cplusplus
}
#endif
#endif
