#include "FunctionBlockLifecycle.h"

#include <stddef.h>

void FunctionBlockLifecycle_Initialize(FunctionBlockLifecycle_t *lifecycle)
{
    if (lifecycle == NULL)
    {
        return;
    }

    lifecycle->state = FUNCTION_BLOCK_STATE_IDLE;
    lifecycle->error_code = 0U;
    lifecycle->previous_execute = false;
}

bool FunctionBlockLifecycle_Begin(FunctionBlockLifecycle_t *lifecycle,
                                  bool enable,
                                  bool execute,
                                  bool reset)
{
    bool rising_edge;

    if (lifecycle == NULL)
    {
        return false;
    }
    if (reset)
    {
        FunctionBlockLifecycle_Initialize(lifecycle);
        lifecycle->previous_execute = execute;
        return false;
    }
    if (!enable)
    {
        lifecycle->state = FUNCTION_BLOCK_STATE_DISABLED;
        lifecycle->error_code = 0U;
        lifecycle->previous_execute = execute;
        return false;
    }
    if (lifecycle->state == FUNCTION_BLOCK_STATE_DISABLED)
    {
        lifecycle->state = FUNCTION_BLOCK_STATE_IDLE;
    }

    rising_edge = (execute && (!lifecycle->previous_execute));
    lifecycle->previous_execute = execute;
    if (rising_edge && (lifecycle->state != FUNCTION_BLOCK_STATE_RUNNING))
    {
        lifecycle->state = FUNCTION_BLOCK_STATE_RUNNING;
        lifecycle->error_code = 0U;
        return true;
    }
    return false;
}

void FunctionBlockLifecycle_Complete(FunctionBlockLifecycle_t *lifecycle)
{
    if ((lifecycle != NULL) &&
        (lifecycle->state == FUNCTION_BLOCK_STATE_RUNNING))
    {
        lifecycle->state = FUNCTION_BLOCK_STATE_DONE;
        lifecycle->error_code = 0U;
    }
}

void FunctionBlockLifecycle_Fail(FunctionBlockLifecycle_t *lifecycle,
                                 uint32_t error_code)
{
    if (lifecycle != NULL)
    {
        lifecycle->state = FUNCTION_BLOCK_STATE_ERROR;
        lifecycle->error_code = error_code;
    }
}

bool FunctionBlockLifecycle_IsBusy(const FunctionBlockLifecycle_t *lifecycle)
{
    return ((lifecycle != NULL) &&
            (lifecycle->state == FUNCTION_BLOCK_STATE_RUNNING));
}
