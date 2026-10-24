#ifndef FB_RECORD_TRIGGER_H
#define FB_RECORD_TRIGGER_H
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum { FB_RECORD_RISING=0, FB_RECORD_FALLING, FB_RECORD_BOTH } FbRecordEdge;
enum { FB_RECORD_REASON_PERIODIC=1u, FB_RECORD_REASON_EVENT=2u };
typedef struct {
    bool periodic_enabled, event_enabled;
    uint32_t period_ms; /* >0 when periodic enabled */
    FbRecordEdge edge;
    uint32_t event_delay_ms, cooldown_ms;
    bool event_one_shot; /* affects events only, not periodic requests */
} FbRecordTriggerConfig;
typedef struct {
    bool enable, run, reset; /* reset level clears entire episode */
    bool event, event_valid;
    uint32_t elapsed_ms;
} FbRecordTriggerInput;
typedef enum { FB_RECORD_OK=0, FB_RECORD_DISABLED, FB_RECORD_STOPPED,
    FB_RECORD_RESET, FB_RECORD_INVALID_ARGUMENT, FB_RECORD_INVALID_CONFIG } FbRecordStatus;
typedef struct {
    bool valid, request;
    uint8_t reasons;
    uint32_t missed_periods, period_elapsed_ms;
    bool event_pending, event_armed, event_fault;
    uint32_t event_elapsed_ms, cooldown_remaining_ms;
    FbRecordStatus status;
} FbRecordTriggerOutput;
typedef struct {
    FbRecordTriggerConfig config;
    bool configured, event_seeded, previous_event, pending, event_fired;
    uint32_t period_elapsed_ms, event_elapsed_ms, cooldown_remaining_ms;
} FbRecordTrigger;
/* Init/zero-init required; no allocation, recording buffer, NVM or hardware.
 * Request is a one-call pulse; reasons may combine. First call/config change
 * starts clocks at zero and seeds event level without generating an edge.
 * Invalid event cancels pending but keeps periodic time and one-shot history.
 * Args must have non-overlapping stable storage. */
void FbRecordTrigger_Init(FbRecordTrigger *instance);
void FbRecordTrigger_DefaultConfig(FbRecordTriggerConfig *config);
bool FbRecordTrigger_ValidateConfig(const FbRecordTriggerConfig *config);
FbRecordStatus FbRecordTrigger_Execute(FbRecordTrigger *instance,
    const FbRecordTriggerConfig *config, const FbRecordTriggerInput *input,
    FbRecordTriggerOutput *output);
#ifdef __cplusplus
}
#endif
#endif
