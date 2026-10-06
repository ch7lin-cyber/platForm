#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "SafetyService.h"
#include "SnapshotService.h"

typedef struct
{
    uint32_t call_count;
    uint32_t last_mask;
    bool last_inhibit;
    bool fail_next;
} MockOutputAction_t;

static bool ApplySafeOutput(bool inhibit,
                            uint32_t trip_source_mask,
                            void *context)
{
    MockOutputAction_t *mock = (MockOutputAction_t *)context;

    assert(mock != NULL);
    mock->call_count++;
    mock->last_mask = trip_source_mask;
    mock->last_inhibit = inhibit;
    if (mock->fail_next)
    {
        mock->fail_next = false;
        return false;
    }
    return true;
}

static void TestNonLatchingTripAndRelease(void)
{
    MockOutputAction_t action = {0};
    SafetyServiceStatus_t status;
    SnapshotRecord_t snapshot;
    int32_t values[SNAPSHOT_SERVICE_VALUE_COUNT] = {2500, 0};

    SnapshotService_Initialize();
    assert(SafetyService_Initialize(0U, ApplySafeOutput, &action));
    assert(SafetyService_UpdateSource(
        SAFETY_SOURCE_LOW_VOLTAGE, true, 7U, 100U, 2U, 3U, values));
    assert(SnapshotService_GetCount() == 1U);
    assert(SnapshotService_GetNewest(0U, &snapshot));
    assert(snapshot.source == SNAPSHOT_SOURCE_SAFETY);
    assert(snapshot.code == SAFETY_SOURCE_LOW_VOLTAGE);
    assert(snapshot.detail == 7U);
    assert(snapshot.values[0] == 2500);

    assert(SafetyService_Process());
    assert(SafetyService_IsOutputInhibited());
    assert(action.call_count == 1U);
    assert(action.last_inhibit);
    assert(action.last_mask == SAFETY_SOURCE_LOW_VOLTAGE);

    assert(SafetyService_UpdateSource(
        SAFETY_SOURCE_LOW_VOLTAGE, false, 0U, 110U, 2U, 3U, NULL));
    assert(SafetyService_Process());
    assert(!SafetyService_IsOutputInhibited());
    assert(action.call_count == 2U);
    assert(!action.last_inhibit);
    assert(SafetyService_GetStatus(&status));
    assert(status.state == SAFETY_STATE_NORMAL);
    assert(status.transition_count == 2U);
}

static void TestLatchingSourceRequiresReset(void)
{
    MockOutputAction_t action = {0};
    SafetyServiceStatus_t status;

    SnapshotService_Initialize();
    assert(SafetyService_Initialize(
        SAFETY_SOURCE_MCU_OVERTEMPERATURE, ApplySafeOutput, &action));
    assert(SafetyService_UpdateSource(
        SAFETY_SOURCE_MCU_OVERTEMPERATURE, true,
        8500U, 200U, 0U, 0U, NULL));
    assert(SafetyService_Process());
    assert(SafetyService_Reset(SAFETY_SOURCE_MCU_OVERTEMPERATURE) ==
           SAFETY_RESET_BLOCKED_ACTIVE);

    assert(SafetyService_UpdateSource(
        SAFETY_SOURCE_MCU_OVERTEMPERATURE, false,
        0U, 210U, 0U, 0U, NULL));
    assert(SafetyService_Process());
    assert(SafetyService_IsOutputInhibited());
    assert(SafetyService_GetStatus(&status));
    assert(status.active_source_mask == 0U);
    assert(status.latched_source_mask ==
           SAFETY_SOURCE_MCU_OVERTEMPERATURE);

    assert(SafetyService_Reset(SAFETY_SOURCE_MCU_OVERTEMPERATURE) ==
           SAFETY_RESET_OK);
    assert(SafetyService_Process());
    assert(!SafetyService_IsOutputInhibited());
}

static void TestMultipleSourcesAndActionRetry(void)
{
    MockOutputAction_t action = {0};
    SafetyServiceStatus_t status;

    SnapshotService_Initialize();
    assert(SafetyService_Initialize(0U, ApplySafeOutput, &action));
    assert(SafetyService_UpdateSource(
        SAFETY_SOURCE_LOW_VOLTAGE | SAFETY_SOURCE_SENSOR_FAULT,
        true, 1U, 300U, 0U, 0U, NULL));
    action.fail_next = true;
    assert(!SafetyService_Process());
    assert(!SafetyService_IsOutputInhibited());
    assert(SafetyService_GetStatus(&status));
    assert(status.state == SAFETY_STATE_ACTION_ERROR);

    assert(SafetyService_Process());
    assert(SafetyService_IsOutputInhibited());
    assert(action.call_count == 2U);
    assert(action.last_mask ==
           (SAFETY_SOURCE_LOW_VOLTAGE | SAFETY_SOURCE_SENSOR_FAULT));

    assert(SafetyService_UpdateSource(
        SAFETY_SOURCE_LOW_VOLTAGE, false, 0U, 310U, 0U, 0U, NULL));
    assert(SafetyService_Process());
    assert(action.call_count == 2U);
    assert(SafetyService_IsOutputInhibited());

    assert(SafetyService_UpdateSource(
        SAFETY_SOURCE_SENSOR_FAULT, false, 0U, 320U, 0U, 0U, NULL));
    assert(SafetyService_Process());
    assert(action.call_count == 3U);
    assert(!SafetyService_IsOutputInhibited());
}

static void TestValidation(void)
{
    SnapshotService_Initialize();
    assert(!SafetyService_Initialize(1UL << 30U, NULL, NULL));
    assert(SafetyService_Initialize(0U, NULL, NULL));
    assert(!SafetyService_UpdateSource(0U, true, 0U, 0U, 0U, 0U, NULL));
    assert(!SafetyService_UpdateSource(
        1UL << 30U, true, 0U, 0U, 0U, 0U, NULL));
    assert(SafetyService_Reset(0U) == SAFETY_RESET_INVALID_ARGUMENT);
    assert(!SafetyService_GetStatus(NULL));
}

int main(void)
{
    TestNonLatchingTripAndRelease();
    TestLatchingSourceRequiresReset();
    TestMultipleSourcesAndActionRetry();
    TestValidation();
    return 0;
}
