#ifndef SYSTEM_ROUTINE_INTERNAL_H
#define SYSTEM_ROUTINE_INTERNAL_H

#include "SystemRoutine.h"

typedef struct
{
    SystemRoutineConfiguration_t configuration;
    SystemRoutineStatus_t status;
} SystemRoutineContext_t;

SystemRoutineContext_t *SystemRoutine_InternalGetContext(void);

bool SystemRoutine_InternalRunMonitor(SystemRoutineMonitor_t monitor,
                                      void *monitor_context,
                                      uint32_t timestamp_ms);

#endif /* SYSTEM_ROUTINE_INTERNAL_H */
