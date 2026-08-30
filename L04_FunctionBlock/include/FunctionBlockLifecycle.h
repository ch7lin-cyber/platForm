#ifndef FUNCTION_BLOCK_LIFECYCLE_H
#define FUNCTION_BLOCK_LIFECYCLE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    FUNCTION_BLOCK_STATE_DISABLED = 0,
    FUNCTION_BLOCK_STATE_IDLE,
    FUNCTION_BLOCK_STATE_RUNNING,
    FUNCTION_BLOCK_STATE_DONE,
    FUNCTION_BLOCK_STATE_ERROR
} FunctionBlockState_t;

typedef struct
{
    FunctionBlockState_t state;
    uint32_t error_code;
    bool previous_execute;
} FunctionBlockLifecycle_t;

void FunctionBlockLifecycle_Initialize(FunctionBlockLifecycle_t *lifecycle);

/* Returns true once on the rising edge of execute while enabled. */
bool FunctionBlockLifecycle_Begin(FunctionBlockLifecycle_t *lifecycle,
                                  bool enable,
                                  bool execute,
                                  bool reset);

void FunctionBlockLifecycle_Complete(FunctionBlockLifecycle_t *lifecycle);
void FunctionBlockLifecycle_Fail(FunctionBlockLifecycle_t *lifecycle,
                                 uint32_t error_code);
bool FunctionBlockLifecycle_IsBusy(const FunctionBlockLifecycle_t *lifecycle);

#ifdef __cplusplus
}
#endif

#endif /* FUNCTION_BLOCK_LIFECYCLE_H */
