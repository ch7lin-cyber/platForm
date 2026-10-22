#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "FaultService.h"
#include "SafetyService.h"
#include "SnapshotService.h"
#include "SystemEventService.h"
#include "SystemFaultService.h"
#include "SystemRoutine.h"
#include "WarningService.h"

typedef struct
{
    uint32_t monitor_calls;
    uint32_t output_calls;
    uint32_t output_source_mask;
    bool output_inhibited;
    bool fail_next_output_action;
} FaultInjectionContext_t;

static bool Monitor(uint32_t timestamp_ms, void *context)
{
    FaultInjectionContext_t *state = (FaultInjectionContext_t *)context;
    int32_t values[SNAPSHOT_SERVICE_VALUE_COUNT] = {0};

    assert(state != NULL);
    state->monitor_calls++;
    values[0] = (int32_t)timestamp_ms;

    switch (state->monitor_calls)
    {
        case 1U:
        case 2U:
            return WarningService_UpdateSource(
                WARNING_SOURCE_MCU_TEMPERATURE, true,
                8400U, timestamp_ms, 3U, 11U, values);

        case 3U:
            assert(SystemFaultService_Raise(
                FAULT_CODE_MCU_OVERTEMPERATURE, 8600U,
                3U, 12U, timestamp_ms, values));
            assert(SafetyService_UpdateSource(
                SAFETY_SOURCE_MCU_OVERTEMPERATURE, true,
                8600U, timestamp_ms, 3U, 12U, values));
            /* Inject a monitor failure while a safety trip is pending. */
            return false;

        case 4U:
            assert(WarningService_UpdateSource(
                WARNING_SOURCE_MCU_TEMPERATURE, false,
                0U, timestamp_ms, 3U, 13U, NULL));
            return SafetyService_UpdateSource(
                SAFETY_SOURCE_MCU_OVERTEMPERATURE, false,
                0U, timestamp_ms, 3U, 13U, NULL);

        case 5U:
            return WarningService_UpdateSource(
                WARNING_SOURCE_MCU_TEMPERATURE, false,
                0U, timestamp_ms, 3U, 13U, NULL);

        default:
            return true;
    }
}

static bool ApplySafeOutput(bool inhibit,
                            uint32_t source_mask,
                            void *context)
{
    FaultInjectionContext_t *state = (FaultInjectionContext_t *)context;

    assert(state != NULL);
    state->output_calls++;
    if (state->fail_next_output_action)
    {
        state->fail_next_output_action = false;
        return false;
    }

    state->output_inhibited = inhibit;
    state->output_source_mask = source_mask;
    return true;
}

static void TestFaultToRecoveryThroughSystemRoutine(void)
{
    const WarningSourceConfiguration_t warning_configuration =
        {2U, 2U, false};
    FaultInjectionContext_t state = {0U, 0U, 0U, false, false};
    SystemRoutineConfiguration_t configuration = {0};
    SystemRoutineStatus_t routine_status;
    SafetyServiceStatus_t safety_status;
    WarningServiceStatus_t warning_status;
    FaultRecord_t fault;
    SnapshotRecord_t snapshot;
    SystemEventRecord_t event;

    configuration.safety_latching_source_mask =
        SAFETY_SOURCE_MCU_OVERTEMPERATURE;
    configuration.safety_output_action = ApplySafeOutput;
    configuration.safety_output_context = &state;
    configuration.fast_monitor = Monitor;
    configuration.fast_monitor_context = &state;

    assert(SystemRoutine_Initialize(&configuration));
    assert(WarningService_ConfigureSource(
        WARNING_SOURCE_MCU_TEMPERATURE, &warning_configuration));

    /* Two consecutive samples assert one warning and capture one snapshot. */
    assert(SystemRoutine_ExecuteFast(100U));
    assert(!WarningService_IsActive(WARNING_SOURCE_MCU_TEMPERATURE));
    assert(SystemRoutine_ExecuteFast(200U));
    assert(WarningService_IsActive(WARNING_SOURCE_MCU_TEMPERATURE));
    assert(SnapshotService_GetCount() == 1U);
    assert(SnapshotService_GetNewest(0U, &snapshot));
    assert(snapshot.source == SNAPSHOT_SOURCE_WARNING);
    assert(snapshot.timestamp_ms == 200U);
    assert(snapshot.values[0] == 200);

    /* Inject an overtemperature fault and trip; safe output is attempted
     * despite monitor failure, and a failed action is retried next cycle. */
    state.fail_next_output_action = true;
    assert(!SystemRoutine_ExecuteFast(300U));
    assert(state.output_calls == 1U);
    assert(!state.output_inhibited);
    assert(SafetyService_GetStatus(&safety_status));
    assert(safety_status.state == SAFETY_STATE_ACTION_ERROR);
    assert(safety_status.latched_source_mask ==
           SAFETY_SOURCE_MCU_OVERTEMPERATURE);
    assert(FaultService_IsActive(FAULT_CODE_MCU_OVERTEMPERATURE));
    assert(SnapshotService_GetCount() == 3U);
    assert(SnapshotService_GetNewest(1U, &snapshot));
    assert(snapshot.source == SNAPSHOT_SOURCE_FAULT);
    assert(snapshot.event_id == 12U);
    assert(SnapshotService_GetNewest(0U, &snapshot));
    assert(snapshot.source == SNAPSHOT_SOURCE_SAFETY);
    assert(snapshot.event_id == 12U);

    assert(SystemRoutine_ExecuteFast(400U));
    assert(state.output_calls == 2U);
    assert(state.output_inhibited);
    assert(state.output_source_mask == SAFETY_SOURCE_MCU_OVERTEMPERATURE);
    assert(SafetyService_IsOutputInhibited());
    assert(SystemRoutine_GetStatus(&routine_status));
    assert(routine_status.fast_failure_count == 1U);
    assert(WarningService_IsActive(WARNING_SOURCE_MCU_TEMPERATURE));

    /* The physical condition clears first; the safety latch and fault remain
     * until warning recovery and an explicit reset are completed. */
    assert(SystemRoutine_ExecuteFast(500U));
    assert(!WarningService_IsActive(WARNING_SOURCE_MCU_TEMPERATURE));
    assert(SystemRoutine_ExecuteFast(600U));
    assert(WarningService_GetStatus(&warning_status));
    assert(warning_status.transition_count == 2U);
    assert(FaultService_IsActive(FAULT_CODE_MCU_OVERTEMPERATURE));
    assert(SafetyService_GetStatus(&safety_status));
    assert(safety_status.active_source_mask == 0U);
    assert(safety_status.latched_source_mask ==
           SAFETY_SOURCE_MCU_OVERTEMPERATURE);

    assert(SystemFaultService_Reset(
        FAULT_CODE_MCU_OVERTEMPERATURE,
        SAFETY_SOURCE_MCU_OVERTEMPERATURE, 700U, 14U) ==
        SYSTEM_FAULT_RESET_OK);
    assert(!FaultService_IsActive(FAULT_CODE_MCU_OVERTEMPERATURE));
    assert(FaultService_Get(FAULT_CODE_MCU_OVERTEMPERATURE, &fault));
    assert(!fault.active);
    assert(SystemRoutine_ExecuteFast(800U));
    assert(!state.output_inhibited);
    assert(!SafetyService_IsOutputInhibited());

    /* Assert/clear/reset transitions remain ordered and correlated. */
    assert(SystemEventService_GetCount() == 7U);
    assert(SystemEventService_GetNewest(0U, &event));
    assert(event.domain == SYSTEM_EVENT_DOMAIN_SAFETY);
    assert(event.state == SYSTEM_EVENT_STATE_RESET);
    assert(event.correlation_event_id == 14U);
    assert(SystemEventService_GetNewest(1U, &event));
    assert(event.domain == SYSTEM_EVENT_DOMAIN_FAULT);
    assert(event.state == SYSTEM_EVENT_STATE_RESET);
    assert(event.correlation_event_id == 14U);
    assert(SystemEventService_GetOldest(0U, &event));
    assert(event.domain == SYSTEM_EVENT_DOMAIN_WARNING);
    assert(event.state == SYSTEM_EVENT_STATE_ASSERTED);
    assert(SystemEventService_GetOldest(6U, &event));
    assert(event.domain == SYSTEM_EVENT_DOMAIN_SAFETY);
    assert(event.state == SYSTEM_EVENT_STATE_RESET);
}

static void TestRepeatedFaultRecoveryUnderLoad(void)
{
    const uint32_t cycle_count = 1000U;
    FaultInjectionContext_t state = {0U, 0U, 0U, false, false};
    SystemRoutineConfiguration_t configuration = {0};
    SystemRoutineStatus_t routine_status;
    FaultRecord_t fault;
    SnapshotRecord_t snapshot;
    SystemEventRecord_t event;
    int32_t values[SNAPSHOT_SERVICE_VALUE_COUNT] = {0};
    uint32_t cycle;

    configuration.safety_latching_source_mask =
        SAFETY_SOURCE_MCU_OVERTEMPERATURE;
    configuration.safety_output_action = ApplySafeOutput;
    configuration.safety_output_context = &state;
    assert(SystemRoutine_Initialize(&configuration));

    for (cycle = 1U; cycle <= cycle_count; cycle++)
    {
        const uint32_t timestamp_ms = cycle * 10U;
        values[0] = (int32_t)cycle;

        assert(SystemFaultService_Raise(
            FAULT_CODE_MCU_OVERTEMPERATURE, 8600U, 3U,
            cycle, timestamp_ms, values));
        assert(SafetyService_UpdateSource(
            SAFETY_SOURCE_MCU_OVERTEMPERATURE, true,
            8600U, timestamp_ms, 3U, cycle, values));
        assert(SystemRoutine_ExecuteFast(timestamp_ms));
        assert(state.output_inhibited);

        assert(SafetyService_UpdateSource(
            SAFETY_SOURCE_MCU_OVERTEMPERATURE, false,
            0U, timestamp_ms + 1U, 3U, cycle, NULL));
        assert(SystemFaultService_Reset(
            FAULT_CODE_MCU_OVERTEMPERATURE,
            SAFETY_SOURCE_MCU_OVERTEMPERATURE,
            timestamp_ms + 2U, cycle) == SYSTEM_FAULT_RESET_OK);
        assert(SystemRoutine_ExecuteFast(timestamp_ms + 3U));
        assert(!state.output_inhibited);
    }

    assert(state.output_calls == (cycle_count * 2U));
    assert(FaultService_Get(FAULT_CODE_MCU_OVERTEMPERATURE, &fault));
    assert(!fault.active);
    assert(FaultService_GetActiveCount() == 0U);
    assert(SystemRoutine_GetStatus(&routine_status));
    assert(routine_status.fast_execution_count == (cycle_count * 2U));
    assert(routine_status.fast_failure_count == 0U);

    /* Sustained transitions wrap both diagnostic rings without corrupting
     * the latest record or the associated fault/safety event ordering. */
    assert(SnapshotService_GetCount() == SNAPSHOT_SERVICE_CAPACITY);
    assert(SnapshotService_GetNewest(0U, &snapshot));
    assert(snapshot.source == SNAPSHOT_SOURCE_SAFETY);
    assert(snapshot.event_id == cycle_count);
    assert(snapshot.values[0] == (int32_t)cycle_count);
    assert(SnapshotService_GetNewest(1U, &snapshot));
    assert(snapshot.source == SNAPSHOT_SOURCE_FAULT);
    assert(snapshot.event_id == cycle_count);

    assert(SystemEventService_GetCount() == SYSTEM_EVENT_SERVICE_CAPACITY);
    assert(SystemEventService_GetNewest(0U, &event));
    assert(event.domain == SYSTEM_EVENT_DOMAIN_SAFETY);
    assert(event.state == SYSTEM_EVENT_STATE_RESET);
    assert(event.correlation_event_id == cycle_count);
    assert(SystemEventService_GetNewest(1U, &event));
    assert(event.domain == SYSTEM_EVENT_DOMAIN_FAULT);
    assert(event.state == SYSTEM_EVENT_STATE_RESET);
    assert(event.correlation_event_id == cycle_count);
}

int main(void)
{
    TestFaultToRecoveryThroughSystemRoutine();
    TestRepeatedFaultRecoveryUnderLoad();
    return 0;
}
