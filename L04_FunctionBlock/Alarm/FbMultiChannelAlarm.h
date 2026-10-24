#ifndef FB_MULTI_CHANNEL_ALARM_H
#define FB_MULTI_CHANNEL_ALARM_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum { FB_MULTI_ALARM_OK=0, FB_MULTI_ALARM_DISABLED,
    FB_MULTI_ALARM_INVALID_ARGUMENT, FB_MULTI_ALARM_INVALID_CONFIG,
    FB_MULTI_ALARM_INVALID_INPUT } FbMultiAlarmStatus;
typedef struct {
    const bool *selected; /* count entries, immutable until revision changes */
    size_t count; /* capacity of selection and input sample arrays */
    uint32_t revision; /* increment when selection entries change */
    uint32_t high_limit; /* 0.001 C for temperature; strict spread > limit */
    uint32_t delay_ms; /* 0..10000, ON delay only */
    bool hold;
} FbMultiChannelAlarmConfig;
typedef struct { int32_t value; bool valid; } FbMultiAlarmSample;
typedef struct {
    const FbMultiAlarmSample *samples;
    size_t count; /* must equal config count */
    uint32_t elapsed_ms;
    bool enable;
    bool reset; /* rising edge; accepted only for healthy normal condition */
} FbMultiChannelAlarmInput;
typedef struct {
    int32_t minimum, maximum;
    uint64_t spread;
    size_t selected_count;
    uint32_t elapsed_ms;
    bool valid;
    bool condition_active;
    bool qualified_active;
    bool latched;
    bool active; /* qualified || latched; stale latch may remain if !valid */
    bool action_request; /* one-call pulse, no action ID or hardware writes */
    bool reset_accepted, reset_rejected;
    FbMultiAlarmStatus status;
} FbMultiChannelAlarmOutput;
typedef struct {
    FbMultiChannelAlarmConfig config;
    uint32_t elapsed_ms;
    bool configured, pending, fired, latched, reset_previous;
} FbMultiChannelAlarm;

/* Init/zero initialization required. No allocation; arrays borrowed per call.
 * Source identity/units/time coherence supplied by caller. Selection edits
 * require a new revision; no table contents retained. Configuration changes
 * start a new episode. Disable clears all state. Invalid data cancels pending
 * timing but preserves fired/latch to prevent duplicate requests on recovery.
 * All pointer storage must be valid, non-overlapping and stable during call. */
void FbMultiChannelAlarm_Init(FbMultiChannelAlarm *instance);
FbMultiAlarmStatus FbMultiChannelAlarm_Execute(FbMultiChannelAlarm *instance,
    const FbMultiChannelAlarmConfig *config,
    const FbMultiChannelAlarmInput *input, FbMultiChannelAlarmOutput *output);
#ifdef __cplusplus
}
#endif
#endif
