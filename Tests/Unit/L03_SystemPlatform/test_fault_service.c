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
    assert(FaultService_Raise(FAULT_CODE_LOW_VOLTAGE, 1U, 2U, 0U));
    assert(FaultService_Get(FAULT_CODE_LOW_VOLTAGE, &record));
    assert(record.last_detail == 1U);
    assert(record.last_configuration_revision == 2U);
    return 0;
}
