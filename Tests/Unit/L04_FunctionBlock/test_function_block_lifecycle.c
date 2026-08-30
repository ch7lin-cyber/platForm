#include <assert.h>

#include "FunctionBlockLifecycle.h"

int main(void)
{
    FunctionBlockLifecycle_t lifecycle;

    FunctionBlockLifecycle_Initialize(&lifecycle);
    assert(lifecycle.state == FUNCTION_BLOCK_STATE_IDLE);
    assert(!FunctionBlockLifecycle_IsBusy(&lifecycle));

    assert(FunctionBlockLifecycle_Begin(&lifecycle, true, true, false));
    assert(FunctionBlockLifecycle_IsBusy(&lifecycle));
    assert(!FunctionBlockLifecycle_Begin(&lifecycle, true, true, false));

    FunctionBlockLifecycle_Complete(&lifecycle);
    assert(lifecycle.state == FUNCTION_BLOCK_STATE_DONE);

    assert(!FunctionBlockLifecycle_Begin(&lifecycle, true, false, false));
    assert(FunctionBlockLifecycle_Begin(&lifecycle, true, true, false));
    FunctionBlockLifecycle_Fail(&lifecycle, 42U);
    assert(lifecycle.state == FUNCTION_BLOCK_STATE_ERROR);
    assert(lifecycle.error_code == 42U);

    assert(!FunctionBlockLifecycle_Begin(&lifecycle, true, false, true));
    assert(lifecycle.state == FUNCTION_BLOCK_STATE_IDLE);

    assert(!FunctionBlockLifecycle_Begin(&lifecycle, false, false, false));
    assert(lifecycle.state == FUNCTION_BLOCK_STATE_DISABLED);
    return 0;
}
