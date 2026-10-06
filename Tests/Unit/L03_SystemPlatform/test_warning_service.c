#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "SnapshotService.h"
#include "WarningService.h"

static void TestAssertAndClearDebounce(void)
{
    const WarningSourceConfiguration_t configuration = {3U, 2U, false};
    WarningServiceStatus_t status;
    SnapshotRecord_t snapshot;
    int32_t values[SNAPSHOT_SERVICE_VALUE_COUNT] = {8100, 0};

    SnapshotService_Initialize();
    WarningService_Initialize();
    assert(WarningService_ConfigureSource(
        WARNING_SOURCE_MCU_TEMPERATURE, &configuration));

    assert(WarningService_UpdateSource(
        WARNING_SOURCE_MCU_TEMPERATURE, true,
        8100U, 1000U, 4U, 7U, values));
    assert(WarningService_UpdateSource(
        WARNING_SOURCE_MCU_TEMPERATURE, true,
        8200U, 2000U, 4U, 7U, values));
    assert(!WarningService_IsActive(WARNING_SOURCE_MCU_TEMPERATURE));
    assert(SnapshotService_GetCount() == 0U);

    assert(WarningService_UpdateSource(
        WARNING_SOURCE_MCU_TEMPERATURE, true,
        8300U, 3000U, 4U, 7U, values));
    assert(WarningService_IsActive(WARNING_SOURCE_MCU_TEMPERATURE));
    assert(SnapshotService_GetCount() == 1U);
    assert(SnapshotService_GetNewest(0U, &snapshot));
    assert(snapshot.source == SNAPSHOT_SOURCE_WARNING);
    assert(snapshot.code == WARNING_SOURCE_MCU_TEMPERATURE);
    assert(snapshot.detail == 8300U);
    assert(snapshot.timestamp_ms == 3000U);
    assert(snapshot.values[0] == 8100);

    assert(WarningService_UpdateSource(
        WARNING_SOURCE_MCU_TEMPERATURE, false,
        0U, 4000U, 4U, 7U, NULL));
    assert(WarningService_IsActive(WARNING_SOURCE_MCU_TEMPERATURE));
    assert(WarningService_UpdateSource(
        WARNING_SOURCE_MCU_TEMPERATURE, false,
        0U, 5000U, 4U, 7U, NULL));
    assert(!WarningService_IsActive(WARNING_SOURCE_MCU_TEMPERATURE));
    assert(WarningService_GetStatus(&status));
    assert(status.transition_count == 2U);
}

static void TestInterruptedDebounceAndLatch(void)
{
    const WarningSourceConfiguration_t configuration = {2U, 1U, true};
    WarningServiceStatus_t status;

    SnapshotService_Initialize();
    WarningService_Initialize();
    assert(WarningService_ConfigureSource(
        WARNING_SOURCE_LOW_VOLTAGE, &configuration));
    assert(WarningService_UpdateSource(
        WARNING_SOURCE_LOW_VOLTAGE, true, 1U, 1U, 0U, 0U, NULL));
    assert(WarningService_UpdateSource(
        WARNING_SOURCE_LOW_VOLTAGE, false, 0U, 2U, 0U, 0U, NULL));
    assert(WarningService_UpdateSource(
        WARNING_SOURCE_LOW_VOLTAGE, true, 1U, 3U, 0U, 0U, NULL));
    assert(!WarningService_IsActive(WARNING_SOURCE_LOW_VOLTAGE));
    assert(WarningService_UpdateSource(
        WARNING_SOURCE_LOW_VOLTAGE, true, 1U, 4U, 0U, 0U, NULL));
    assert(WarningService_IsActive(WARNING_SOURCE_LOW_VOLTAGE));
    assert(!WarningService_Reset(WARNING_SOURCE_LOW_VOLTAGE));

    assert(WarningService_UpdateSource(
        WARNING_SOURCE_LOW_VOLTAGE, false, 0U, 5U, 0U, 0U, NULL));
    assert(WarningService_GetStatus(&status));
    assert(status.active_source_mask == 0U);
    assert(status.latched_source_mask == WARNING_SOURCE_LOW_VOLTAGE);
    assert(WarningService_IsActive(WARNING_SOURCE_LOW_VOLTAGE));
    assert(WarningService_Reset(WARNING_SOURCE_LOW_VOLTAGE));
    assert(!WarningService_IsActive(WARNING_SOURCE_LOW_VOLTAGE));
}

static void TestValidation(void)
{
    const WarningSourceConfiguration_t valid = {1U, 1U, false};
    const WarningSourceConfiguration_t invalid = {0U, 1U, false};

    WarningService_Initialize();
    assert(!WarningService_ConfigureSource(0U, &valid));
    assert(!WarningService_ConfigureSource(
        WARNING_SOURCE_COMMUNICATION | WARNING_SOURCE_NVM, &valid));
    assert(!WarningService_ConfigureSource(
        WARNING_SOURCE_COMMUNICATION, &invalid));
    assert(!WarningService_ConfigureSource(
        WARNING_SOURCE_COMMUNICATION, NULL));
    assert(!WarningService_UpdateSource(
        1UL << 30U, true, 0U, 0U, 0U, 0U, NULL));
    assert(!WarningService_GetStatus(NULL));
}

int main(void)
{
    TestAssertAndClearDebounce();
    TestInterruptedDebounceAndLatch();
    TestValidation();
    return 0;
}
