#include "SystemRoutine.h"

#include "SystemRoutineInternal.h"

bool SystemRoutine_ExecuteBackground(uint32_t timestamp_ms)
{
    SystemRoutineContext_t *routine = SystemRoutine_InternalGetContext();
    bool result;

    if (!routine->status.initialized)
    {
        return false;
    }

    routine->status.background_execution_count++;
    result = SystemRoutine_InternalRunMonitor(
        routine->configuration.background_monitor,
        routine->configuration.background_monitor_context,
        timestamp_ms);
    if (!result)
    {
        routine->status.background_failure_count++;
    }
    return result;
}
