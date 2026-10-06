#include "SystemRoutine.h"

#include <stddef.h>
#include <string.h>

#include "FaultService.h"
#include "SnapshotService.h"
#include "SystemEventService.h"
#include "SystemRoutineInternal.h"
#include "WarningService.h"

static SystemRoutineContext_t g_system_routine;

bool SystemRoutine_Initialize(
    const SystemRoutineConfiguration_t *configuration)
{
    if (configuration == NULL)
    {
        return false;
    }

    (void)memset(&g_system_routine, 0, sizeof(g_system_routine));
    SystemEventService_Initialize();
    SnapshotService_Initialize();
    FaultService_Initialize();
    WarningService_Initialize();
    if (!SafetyService_Initialize(
            configuration->safety_latching_source_mask,
            configuration->safety_output_action,
            configuration->safety_output_context))
    {
        return false;
    }

    g_system_routine.configuration = *configuration;
    g_system_routine.status.initialized = true;
    return true;
}

bool SystemRoutine_GetStatus(SystemRoutineStatus_t *status)
{
    if ((!g_system_routine.status.initialized) || (status == NULL))
    {
        return false;
    }
    *status = g_system_routine.status;
    return true;
}

SystemRoutineContext_t *SystemRoutine_InternalGetContext(void)
{
    return &g_system_routine;
}

bool SystemRoutine_InternalRunMonitor(SystemRoutineMonitor_t monitor,
                                      void *monitor_context,
                                      uint32_t timestamp_ms)
{
    if (!g_system_routine.status.initialized)
    {
        return false;
    }
    if (monitor == NULL)
    {
        return true;
    }
    return monitor(timestamp_ms, monitor_context);
}
