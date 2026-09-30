#include <assert.h>

#include "FaultService.h"

int main(void)
{
    FaultRecord_t record;

    FaultService_Initialize();
    assert(!FaultService_IsActive(FAULT_CODE_MCU_OVERTEMPERATURE));
    assert(FaultService_Raise(
        FAULT_CODE_MCU_OVERTEMPERATURE, 8500U, 0U, 0U));
    assert(FaultService_IsActive(FAULT_CODE_MCU_OVERTEMPERATURE));
    assert(FaultService_Get(FAULT_CODE_MCU_OVERTEMPERATURE, &record));
    assert(record.first_detail == 8500U);
    assert(record.last_detail == 8500U);
    assert(record.occurrence_count == 1U);
    assert(FaultService_Clear(FAULT_CODE_MCU_OVERTEMPERATURE));
    assert(!FaultService_IsActive(FAULT_CODE_MCU_OVERTEMPERATURE));
    return 0;
}
