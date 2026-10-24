#ifndef FB_INPUT_SWITCH_OVER_H
#define FB_INPUT_SWITCH_OVER_H
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef enum { FB_SWITCH_SOURCE1=0, FB_SWITCH_SOURCE2=1,
               FB_SWITCH_BOTH=2, FB_SWITCH_NONE=3 } FbSwitchSource;
typedef enum { FB_SWITCH_USE_OTHER=0, FB_SWITCH_KEEP_SOURCE=1 } FbSwitchFailAction;
typedef enum { FB_SWITCH_OK=0, FB_SWITCH_DISABLED, FB_SWITCH_RESET,
    FB_SWITCH_INVALID_ARGUMENT, FB_SWITCH_INVALID_CONFIG,
    FB_SWITCH_INVALID_INPUT } FbSwitchStatus;
typedef struct {
    int32_t sensor_low, sensor_high;
    int32_t switch_low, switch_high;
    FbSwitchFailAction fail_action;
} FbInputSwitchOverConfig;
typedef struct {
    int32_t value[2]; /* same units; temperatures in 0.001 degree C */
    bool valid[2]; /* caller resolves stale/status/unit mismatch before call */
    bool enable;
    bool reset; /* level-sensitive: clear state, zero/invalid this call */
} FbInputSwitchOverInput;
typedef struct {
    int32_t value;
    bool valid;
    bool degraded;
    uint8_t error_mask; /* bit0/1: invalid or out-of-sensor-range source */
    FbSwitchSource selected;
    FbSwitchStatus status;
} FbInputSwitchOverOutput;
typedef struct {
    FbInputSwitchOverConfig config;
    FbSwitchSource selected;
    bool configured;
} FbInputSwitchOver;

/* No allocation/global state. Zero initialization or Init is required.
 * Inclusive sensor limits; invalid source is never averaged or reused as PV.
 * KEEP_SOURCE starts at SOURCE1 if first enabled call has a failed source.
 * Config changes reset history before selection; two healthy sources always
 * re-evaluate selection. Average uses int64 sum, division toward zero.
 * Arguments/storage must be valid and non-overlapping for the call. */
void FbInputSwitchOver_Init(FbInputSwitchOver *instance);
FbSwitchStatus FbInputSwitchOver_Execute(FbInputSwitchOver *instance,
    const FbInputSwitchOverConfig *config, const FbInputSwitchOverInput *input,
    FbInputSwitchOverOutput *output);
#ifdef __cplusplus
}
#endif
#endif
