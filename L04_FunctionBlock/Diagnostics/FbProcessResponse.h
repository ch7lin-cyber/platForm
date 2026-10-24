#ifndef FB_PROCESS_RESPONSE_H
#define FB_PROCESS_RESPONSE_H
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    int32_t final_pv, step_mv; /* known references, not automatic estimates */
    uint32_t mv_tolerance, minimum_mv_step, minimum_pv_span;
    uint32_t onset_change, settling_band, settling_dwell_ms;
    uint32_t minimum_settling_samples, timeout_ms;
} FbProcessResponseConfig;
typedef enum { FB_RESPONSE_CMD_NONE=0, FB_RESPONSE_CMD_START,
    FB_RESPONSE_CMD_STOP, FB_RESPONSE_CMD_RESET } FbResponseCommand;
typedef struct {
    int32_t pv, mv;
    int32_t baseline_pv, baseline_mv; /* sampled only on START */
    uint32_t sample_sequence, elapsed_ms;
    bool valid, baseline_valid, enable;
    FbResponseCommand command;
} FbProcessResponseInput;
typedef enum { FB_RESPONSE_IDLE=0, FB_RESPONSE_OBSERVING, FB_RESPONSE_COMPLETED,
    FB_RESPONSE_CANCELLED, FB_RESPONSE_FAULT } FbResponsePhase;
typedef enum { FB_RESPONSE_OK=0, FB_RESPONSE_DISABLED, FB_RESPONSE_INVALID_ARGUMENT,
    FB_RESPONSE_INVALID_CONFIG, FB_RESPONSE_INVALID_INPUT,
    FB_RESPONSE_EXPERIMENT_CHANGED, FB_RESPONSE_TIMEOUT,
    FB_RESPONSE_COUNTER_OVERFLOW } FbResponseStatus;
typedef struct {
    uint64_t elapsed_ms, samples;
    uint64_t onset_ms, t10_ms, t632_ms, t90_ms, settling_ms;
    bool onset_seen, t10_seen, t632_seen, t90_seen;
    uint64_t overshoot; /* beyond reference final PV, directional */
    double final_mean, reference_gain, measured_gain;
    bool result_valid; /* only completed under all observation conditions */
    FbResponsePhase phase;
    FbResponseStatus status;
} FbProcessResponseOutput;
typedef struct {
    FbProcessResponseConfig config;
    FbProcessResponseOutput metrics;
    int32_t baseline_pv;
    int64_t mv_step, pv_span;
    uint32_t previous_sequence;
    uint64_t band_start_ms, band_samples;
    double band_mean;
    bool has_sequence, in_band;
    FbResponseCommand previous_command;
} FbProcessResponse;
/* Init/zero-init required. No actuation, PID tuning, allocation or NVM.
 * Same sequence must mean same PV sample; caller validates freshness and
 * PV/MV time coherence. New samples only; crossings observed, not backdated.
 * Config fixed until Reset/disable. Inputs/config/state/output may not overlap. */
void FbProcessResponse_Init(FbProcessResponse *instance);
bool FbProcessResponse_ValidateConfig(const FbProcessResponseConfig *config);
FbResponseStatus FbProcessResponse_Execute(FbProcessResponse *instance,
    const FbProcessResponseConfig *config, const FbProcessResponseInput *input,
    FbProcessResponseOutput *output);
#ifdef __cplusplus
}
#endif
#endif
