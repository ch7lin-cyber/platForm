#ifndef FB_BATCH_METRICS_H
#define FB_BATCH_METRICS_H
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum { FB_BATCH_HEATING=0, FB_BATCH_COOLING } FbBatchDirection;
typedef struct {
    uint32_t arrival_band, arrival_dwell_ms;
    FbBatchDirection direction;
} FbBatchMetricsConfig;
typedef enum { FB_BATCH_CMD_NONE=0, FB_BATCH_CMD_START,
    FB_BATCH_CMD_STOP, FB_BATCH_CMD_RESET } FbBatchCommand;
typedef struct {
    int32_t pv, sv; /* 0.001 C; SV captured on START */
    uint32_t sample_sequence, elapsed_ms;
    bool pv_valid, sv_valid, enable;
    FbBatchCommand command; /* command changes create edges */
} FbBatchMetricsInput;
typedef enum { FB_BATCH_IDLE=0, FB_BATCH_RUNNING, FB_BATCH_COMPLETED,
    FB_BATCH_FAULT } FbBatchPhase;
typedef enum { FB_BATCH_OK=0, FB_BATCH_DISABLED, FB_BATCH_INVALID_ARGUMENT,
    FB_BATCH_INVALID_CONFIG, FB_BATCH_INVALID_INPUT, FB_BATCH_COUNTER_OVERFLOW } FbBatchStatus;
typedef struct {
    int32_t target, minimum, maximum;
    double mean, population_variance;
    uint64_t samples, invalid_calls, elapsed_ms, integrated_ms;
    uint64_t overshoot; /* positive directional PV-SV, same PV units */
    double signed_error_integral, absolute_error_integral; /* PV units * seconds */
    bool valid, reached, finished;
    uint64_t arrival_ms;
    FbBatchPhase phase;
    FbBatchStatus status;
} FbBatchMetricsOutput;
typedef struct {
    FbBatchMetricsConfig config;
    FbBatchMetricsOutput metrics;
    double m2;
    int32_t previous_pv;
    uint32_t previous_sequence;
    uint64_t dwell_ms;
    bool previous_valid, previous_in_band, has_sequence;
    FbBatchCommand previous_command;
} FbBatchMetrics;
/* Zero-init/Init required. No allocation, NVM or hardware. Non-overlapping
 * stable arguments required. Duplicate sequence excludes count/mean/extrema,
 * but a still-valid held sample may cover elapsed integration/dwell time.
 * Caller owns freshness validation; invalid intervals never bridge gaps. */
void FbBatchMetrics_Init(FbBatchMetrics *instance);
FbBatchStatus FbBatchMetrics_Execute(FbBatchMetrics *instance,
    const FbBatchMetricsConfig *config, const FbBatchMetricsInput *input,
    FbBatchMetricsOutput *output);
#ifdef __cplusplus
}
#endif
#endif
