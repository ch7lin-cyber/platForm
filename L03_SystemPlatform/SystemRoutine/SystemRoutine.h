#ifndef SYSTEM_ROUTINE_H
#define SYSTEM_ROUTINE_H

#include <stdbool.h>
#include <stdint.h>

#include "SafetyService.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*SystemRoutineMonitor_t)(uint32_t timestamp_ms,
                                       void *context);

typedef struct
{
    uint32_t safety_latching_source_mask;
    SafetyOutputAction_t safety_output_action;
    void *safety_output_context;
    SystemRoutineMonitor_t fast_monitor;
    void *fast_monitor_context;
    SystemRoutineMonitor_t control_monitor;
    void *control_monitor_context;
    SystemRoutineMonitor_t background_monitor;
    void *background_monitor_context;
} SystemRoutineConfiguration_t;

typedef struct
{
    uint32_t fast_execution_count;
    uint32_t fast_failure_count;
    uint32_t control_execution_count;
    uint32_t control_failure_count;
    uint32_t background_execution_count;
    uint32_t background_failure_count;
    bool initialized;
} SystemRoutineStatus_t;

bool SystemRoutine_Initialize(
    const SystemRoutineConfiguration_t *configuration);

bool SystemRoutine_ExecuteFast(uint32_t timestamp_ms);
bool SystemRoutine_ExecuteControl(uint32_t timestamp_ms);
bool SystemRoutine_ExecuteBackground(uint32_t timestamp_ms);

bool SystemRoutine_GetStatus(SystemRoutineStatus_t *status);

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_ROUTINE_H */
