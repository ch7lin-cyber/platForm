#ifndef FB_PID_GROUP_SELECTOR_H
#define FB_PID_GROUP_SELECTOR_H
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define FB_PID_GROUP_COUNT 4u
#define FB_PID_GROUP_AUTO 4u
typedef struct {
    uint32_t pb, cool_pb; /* temperature: 0.001 C; must be positive */
    uint32_t ti_ms, td_ms, cool_ti_ms, cool_td_ms;
    uint16_t integral_preload; /* 0.1%, 0..1000 */
    uint16_t alpha, beta, gamma; /* 0.01, project range 0..200 */
} FbPidGroupParameters;
typedef struct {
    FbPidGroupParameters groups[FB_PID_GROUP_COUNT];
    int32_t switch_sv[FB_PID_GROUP_COUNT-1]; /* non-decreasing, 0.001 C */
    uint8_t selection; /* 0..3 fixed, 4=Auto */
} FbPidGroupSelectorConfig;
typedef struct {
    int32_t sv;
    bool sv_valid;
    bool enable;
    bool lock_group; /* outer AT coordinator supplies explicit target */
    uint8_t locked_group; /* 0..3 when lock_group */
} FbPidGroupSelectorInput;
typedef enum { FB_PID_GROUP_OK=0, FB_PID_GROUP_DISABLED,
    FB_PID_GROUP_INVALID_ARGUMENT, FB_PID_GROUP_INVALID_CONFIG,
    FB_PID_GROUP_INVALID_INPUT } FbPidGroupStatus;
typedef struct {
    FbPidGroupParameters parameters; /* candidate, not backend acknowledgement */
    uint8_t group; /* 0..3 only if valid */
    bool valid;
    bool locked;
    FbPidGroupStatus status;
} FbPidGroupSelectorOutput;

/* Stateless, fixed-size single-channel bank. No allocation, CKLIB, storage,
 * hardware or retained pointers. Caller owns immutable bank during execution.
 * Entire bank is validated before returning any group. Do not overlap args.
 * No reset needed: disable => zero/invalid; lock is a level, not a command.
 * Equal thresholds allowed: Auto may skip empty intervals. No hysteresis. */
void FbPidGroupSelector_DefaultConfig(FbPidGroupSelectorConfig *config);
bool FbPidGroupSelector_ValidateConfig(const FbPidGroupSelectorConfig *config);
FbPidGroupStatus FbPidGroupSelector_Execute(const FbPidGroupSelectorConfig *config,
    const FbPidGroupSelectorInput *input, FbPidGroupSelectorOutput *output);
#ifdef __cplusplus
}
#endif
#endif
