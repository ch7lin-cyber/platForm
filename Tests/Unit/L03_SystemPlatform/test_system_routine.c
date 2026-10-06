#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "FaultService.h"
#include "SafetyService.h"
#include "SnapshotService.h"
#include "SystemEventService.h"
#include "SystemRoutine.h"
#include "WarningService.h"

typedef struct
{
    uint32_t call_count;
    uint32_t timestamp_ms;
    bool result;
} MonitorMock_t;

typedef struct
{
    uint32_t call_count;
    uint32_t source_mask;
    bool inhibited;
} OutputMock_t;

static bool Monitor(uint32_t timestamp_ms, void *context)
{
    MonitorMock_t *mock = (MonitorMock_t *)context;

    assert(mock != NULL);
    mock->call_count++;
    mock->timestamp_ms = timestamp_ms;
    return mock->result;
}

static bool RaiseLowVoltage(uint32_t timestamp_ms, void *context)
{
    assert(Monitor(timestamp_ms, context));
    return SafetyService_UpdateSource(
        SAFETY_SOURCE_LOW_VOLTAGE, true, 1U,
        timestamp_ms, 0U, 0U, NULL);
}

static bool ApplySafeOutput(bool inhibit,
                            uint32_t source_mask,
                            void *context)
{
    OutputMock_t *mock = (OutputMock_t *)context;

    assert(mock != NULL);
    mock->call_count++;
    mock->source_mask = source_mask;
    mock->inhibited = inhibit;
    return true;
}

static void TestInitializationAndExecution(void)
{
    MonitorMock_t fast = {0U, 0U, true};
    MonitorMock_t control = {0U, 0U, true};
    MonitorMock_t background = {0U, 0U, true};
    OutputMock_t output = {0U, 0U, false};
    SystemRoutineConfiguration_t configuration = {0};
    SystemRoutineStatus_t status;

    configuration.safety_latching_source_mask =
        SAFETY_SOURCE_LOW_VOLTAGE;
    configuration.safety_output_action = ApplySafeOutput;
    configuration.safety_output_context = &output;
    configuration.fast_monitor = RaiseLowVoltage;
    configuration.fast_monitor_context = &fast;
    configuration.control_monitor = Monitor;
    configuration.control_monitor_context = &control;
    configuration.background_monitor = Monitor;
    configuration.background_monitor_context = &background;

    assert(SystemRoutine_Initialize(&configuration));
    assert(SnapshotService_GetCount() == 0U);
    assert(SystemEventService_GetCount() == 0U);
    assert(FaultService_GetActiveCount() == 0U);
    assert(!WarningService_IsActive(WARNING_SOURCE_ALL));

    assert(SystemRoutine_ExecuteFast(10U));
    assert(fast.call_count == 1U);
    assert(fast.timestamp_ms == 10U);
    assert(output.call_count == 1U);
    assert(output.inhibited);
    assert(output.source_mask == SAFETY_SOURCE_LOW_VOLTAGE);
    assert(SnapshotService_GetCount() == 1U);
    assert(SystemEventService_GetCount() == 1U);
    assert(SystemRoutine_ExecuteFast(11U));
    assert(output.call_count == 1U);
    assert(SnapshotService_GetCount() == 1U);
    assert(SystemEventService_GetCount() == 1U);

    assert(SystemRoutine_ExecuteControl(20U));
    assert(SystemRoutine_ExecuteBackground(30U));
    assert(control.call_count == 1U);
    assert(control.timestamp_ms == 20U);
    assert(background.call_count == 1U);
    assert(background.timestamp_ms == 30U);

    assert(SystemRoutine_GetStatus(&status));
    assert(status.initialized);
    assert(status.fast_execution_count == 2U);
    assert(status.control_execution_count == 1U);
    assert(status.background_execution_count == 1U);
    assert(status.fast_failure_count == 0U);
}

static void TestFailureAccountingAndOptionalHooks(void)
{
    MonitorMock_t control = {0U, 0U, false};
    SystemRoutineConfiguration_t configuration = {0};
    SystemRoutineStatus_t status;

    configuration.control_monitor = Monitor;
    configuration.control_monitor_context = &control;
    assert(SystemRoutine_Initialize(&configuration));
    assert(SystemRoutine_ExecuteFast(1U));
    assert(!SystemRoutine_ExecuteControl(2U));
    assert(SystemRoutine_ExecuteBackground(3U));
    assert(SystemRoutine_GetStatus(&status));
    assert(status.fast_failure_count == 0U);
    assert(status.control_failure_count == 1U);
    assert(status.background_failure_count == 0U);
}

static void TestValidation(void)
{
    assert(!SystemRoutine_Initialize(NULL));
    assert(!SystemRoutine_ExecuteFast(0U));
    assert(!SystemRoutine_ExecuteControl(0U));
    assert(!SystemRoutine_ExecuteBackground(0U));
    assert(!SystemRoutine_GetStatus(NULL));
}

int main(void)
{
    TestValidation();
    TestInitializationAndExecution();
    TestFailureAccountingAndOptionalHooks();
    return 0;
}
