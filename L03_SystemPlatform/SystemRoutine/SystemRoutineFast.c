#include "SystemRoutine.h"

#include "SystemRoutineInternal.h"

bool SystemRoutine_ExecuteFast(uint32_t timestamp_ms)
{
    SystemRoutineContext_t *routine = SystemRoutine_InternalGetContext();
    bool monitor_result;
    bool safety_result;
    bool result;

    if (!routine->status.initialized)
    {
        return false;
    }

    routine->status.fast_execution_count++;
    monitor_result = SystemRoutine_InternalRunMonitor(
        routine->configuration.fast_monitor,
        routine->configuration.fast_monitor_context,
        timestamp_ms);

    /* Always apply the aggregate safety state, even if monitoring failed. */
    safety_result = SafetyService_Process();
    result = monitor_result && safety_result;
    if (!result)
    {
        routine->status.fast_failure_count++;
    }
    return result;
}
