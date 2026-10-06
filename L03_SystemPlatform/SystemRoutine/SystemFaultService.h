#ifndef SYSTEM_FAULT_SERVICE_H
#define SYSTEM_FAULT_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

#include "FaultService.h"
#include "SnapshotService.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    SYSTEM_FAULT_RESET_OK = 0,
    SYSTEM_FAULT_RESET_NOT_ACTIVE,
    SYSTEM_FAULT_RESET_CONDITION_ACTIVE,
    SYSTEM_FAULT_RESET_INVALID_ARGUMENT,
    SYSTEM_FAULT_RESET_FAILED
} SystemFaultResetResult_t;

/*
 * Raises or updates a FaultService record. Runtime Event and Snapshot records
 * are created only on the inactive-to-active transition.
 */
bool SystemFaultService_Raise(
    FaultCode_t code,
    uint16_t detail,
    uint16_t configuration_revision,
    uint32_t correlation_event_id,
    uint32_t timestamp_ms,
    const int32_t values[SNAPSHOT_SERVICE_VALUE_COUNT]);

/* Clears a recoverable non-safety fault and records a CLEARED event. */
bool SystemFaultService_ClearRecovered(
    FaultCode_t code,
    uint32_t timestamp_ms,
    uint32_t correlation_event_id);

/*
 * Clears a fault and its associated SafetyService latch atomically from the
 * caller's perspective. Reset is rejected while the physical safety source
 * remains active. Pass zero for faults without an associated safety source.
 */
SystemFaultResetResult_t SystemFaultService_Reset(
    FaultCode_t code,
    uint32_t safety_source_mask,
    uint32_t timestamp_ms,
    uint32_t correlation_event_id);

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_FAULT_SERVICE_H */
