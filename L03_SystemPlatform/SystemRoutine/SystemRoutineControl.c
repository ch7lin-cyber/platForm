#include "SystemRoutine.h"

#include "SystemRoutineInternal.h"

bool SystemRoutine_ExecuteControl(uint32_t timestamp_ms)
{
    SystemRoutineContext_t *routine = SystemRoutine_InternalGetContext();
    bool result;

    if (!routine->status.initialized)
    {
        return false;
    }

    routine->status.control_execution_count++;
    result = SystemRoutine_InternalRunMonitor(
        routine->configuration.control_monitor,
        routine->configuration.control_monitor_context,
        timestamp_ms);
    if (!result)
    {
        routine->status.control_failure_count++;
    }
    return result;
}
