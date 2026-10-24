#ifndef FB_CASCADE_COORDINATOR_H
#define FB_CASCADE_COORDINATOR_H
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum { FB_CASCADE_ABSOLUTE=0, FB_CASCADE_DEVIATION } FbCascadeMode;
typedef enum { FB_CASCADE_OK=0, FB_CASCADE_DISABLED, FB_CASCADE_RESET,
    FB_CASCADE_INVALID_ARGUMENT, FB_CASCADE_INVALID_CONFIG,
    FB_CASCADE_INVALID_INPUT } FbCascadeStatus;
typedef struct {
    FbCascadeMode mode;
    int32_t mv_low, mv_high; /* e.g. 0..1000 in 0.1% */
    int32_t sv_low, sv_high; /* absolute limits or offsets from master SV */
    int32_t offset; /* 0.001 C, added after mapping, before clamp/filter */
    uint32_t filter_tau_ms; /* 0 bypass; not DTDM04 Filter Num */
    bool reverse_mapping; /* true maps mv_low to SV high */
} FbCascadeConfig;
typedef struct {
    int32_t master_mv, master_sv;
    bool mv_valid, master_sv_valid;
    uint32_t elapsed_ms;
    bool enable, reset; /* reset level => zero/invalid this call */
} FbCascadeInput;
typedef struct {
    int32_t slave_sv, target_sv, limit_low, limit_high;
    bool valid, mv_clamped, target_clamped, filter_clamped;
    bool at_low, at_high; /* diagnostics, not anti-windup feedback control */
    FbCascadeStatus status;
} FbCascadeOutput;
typedef struct {
    FbCascadeConfig config;
    double filtered;
    bool configured, seeded;
} FbCascadeCoordinator;
/* Single signal, no allocation/PID/hardware. Init/zero-init required.
 * First valid target seeds filter; config change/invalid input clears it.
 * Mapping -> offset -> clamp -> filter -> final clamp, SV truncates to zero.
 * Config/input/output/state must not overlap; stable for call duration. */
void FbCascadeCoordinator_Init(FbCascadeCoordinator *instance);
void FbCascadeCoordinator_DefaultConfig(FbCascadeConfig *config);
bool FbCascadeCoordinator_ValidateConfig(const FbCascadeConfig *config);
FbCascadeStatus FbCascadeCoordinator_Execute(FbCascadeCoordinator *instance,
    const FbCascadeConfig *config, const FbCascadeInput *input, FbCascadeOutput *output);
#ifdef __cplusplus
}
#endif
#endif
