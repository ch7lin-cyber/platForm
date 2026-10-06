#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "FaultService.h"
#include "SafetyService.h"
#include "SnapshotService.h"
#include "SystemEventService.h"
#include "SystemFaultService.h"

static void Initialize(void)
{
    FaultService_Initialize();
    SnapshotService_Initialize();
    SystemEventService_Initialize();
    assert(SafetyService_Initialize(
        SAFETY_SOURCE_MCU_OVERTEMPERATURE, NULL, NULL));
}

static void TestRaiseRecordsOnlyFirstTransition(void)
{
    FaultRecord_t fault;
    SnapshotRecord_t snapshot;
    SystemEventRecord_t event;
    int32_t values[SNAPSHOT_SERVICE_VALUE_COUNT] = {8512, 0};

    Initialize();
    assert(SystemFaultService_Raise(
        FAULT_CODE_MCU_OVERTEMPERATURE, 8512U, 2U, 7U, 100U, values));
    assert(SystemFaultService_Raise(
        FAULT_CODE_MCU_OVERTEMPERATURE, 8600U, 2U, 8U, 200U, values));
    assert(FaultService_Get(FAULT_CODE_MCU_OVERTEMPERATURE, &fault));
    assert(fault.occurrence_count == 2U);
    assert(fault.last_detail == 8600U);
    assert(SystemEventService_GetCount() == 1U);
    assert(SnapshotService_GetCount() == 1U);
    assert(SystemEventService_GetNewest(0U, &event));
    assert(event.domain == SYSTEM_EVENT_DOMAIN_FAULT);
    assert(event.state == SYSTEM_EVENT_STATE_ASSERTED);
    assert(event.timestamp_ms == 100U);
    assert(SnapshotService_GetNewest(0U, &snapshot));
    assert(snapshot.source == SNAPSHOT_SOURCE_FAULT);
    assert(snapshot.values[0] == 8512);
}

static void TestResetRejectsActiveCondition(void)
{
    Initialize();
    assert(SystemFaultService_Raise(
        FAULT_CODE_MCU_OVERTEMPERATURE, 8500U, 0U, 0U,
        10U, NULL));
    assert(SafetyService_UpdateSource(
        SAFETY_SOURCE_MCU_OVERTEMPERATURE, true,
        8500U, 10U, 0U, 0U, NULL));
    assert(SystemFaultService_Reset(
        FAULT_CODE_MCU_OVERTEMPERATURE,
        SAFETY_SOURCE_MCU_OVERTEMPERATURE, 20U, 1U) ==
        SYSTEM_FAULT_RESET_CONDITION_ACTIVE);
    assert(FaultService_IsActive(FAULT_CODE_MCU_OVERTEMPERATURE));
}

static void TestRecoverableFaultClear(void)
{
    SystemEventRecord_t event;

    Initialize();
    assert(SystemFaultService_Raise(
        FAULT_CODE_ANALOG_INPUT_RECONFIGURE_FAILED,
        3U, 4U, 5U, 10U, NULL));
    assert(SystemFaultService_ClearRecovered(
        FAULT_CODE_ANALOG_INPUT_RECONFIGURE_FAILED, 20U, 6U));
    assert(!FaultService_IsActive(
        FAULT_CODE_ANALOG_INPUT_RECONFIGURE_FAILED));
    assert(SystemEventService_GetNewest(0U, &event));
    assert(event.domain == SYSTEM_EVENT_DOMAIN_FAULT);
    assert(event.state == SYSTEM_EVENT_STATE_CLEARED);
    assert(event.timestamp_ms == 20U);
    assert(event.correlation_event_id == 6U);
    assert(SystemFaultService_ClearRecovered(
        FAULT_CODE_ANALOG_INPUT_RECONFIGURE_FAILED, 30U, 7U));
    assert(SystemEventService_GetCount() == 2U);
}

static void TestResetClearsFaultAndLatch(void)
{
    SafetyServiceStatus_t safety;
    SystemEventRecord_t event;

    Initialize();
    assert(SystemFaultService_Raise(
        FAULT_CODE_MCU_OVERTEMPERATURE, 8500U, 0U, 0U,
        10U, NULL));
    assert(SafetyService_UpdateSource(
        SAFETY_SOURCE_MCU_OVERTEMPERATURE, true,
        8500U, 10U, 0U, 0U, NULL));
    assert(SafetyService_UpdateSource(
        SAFETY_SOURCE_MCU_OVERTEMPERATURE, false,
        0U, 20U, 0U, 0U, NULL));
    assert(SystemFaultService_Reset(
        FAULT_CODE_MCU_OVERTEMPERATURE,
        SAFETY_SOURCE_MCU_OVERTEMPERATURE, 30U, 9U) ==
        SYSTEM_FAULT_RESET_OK);
    assert(!FaultService_IsActive(FAULT_CODE_MCU_OVERTEMPERATURE));
    assert(SafetyService_GetStatus(&safety));
    assert((safety.latched_source_mask &
            SAFETY_SOURCE_MCU_OVERTEMPERATURE) == 0U);
    assert(SystemEventService_GetNewest(0U, &event));
    assert(event.domain == SYSTEM_EVENT_DOMAIN_SAFETY);
    assert(event.state == SYSTEM_EVENT_STATE_RESET);
    assert(event.correlation_event_id == 9U);
    assert(SystemEventService_GetNewest(1U, &event));
    assert(event.domain == SYSTEM_EVENT_DOMAIN_FAULT);
    assert(event.state == SYSTEM_EVENT_STATE_RESET);
}

int main(void)
{
    TestRaiseRecordsOnlyFirstTransition();
    TestRecoverableFaultClear();
    TestResetRejectsActiveCondition();
    TestResetClearsFaultAndLatch();
    return 0;
}
