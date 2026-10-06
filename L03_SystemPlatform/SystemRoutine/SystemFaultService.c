#include "SystemFaultService.h"

#include <stddef.h>
#include <string.h>

#include "SafetyService.h"
#include "SystemEventService.h"

static void RecordFaultEvent(FaultCode_t code,
                             SystemEventState_t state,
                             uint16_t detail,
                             uint16_t configuration_revision,
                             uint32_t correlation_event_id,
                             uint32_t timestamp_ms)
{
    SystemEventCapture_t capture;

    (void)memset(&capture, 0, sizeof(capture));
    capture.timestamp_ms = timestamp_ms;
    capture.domain = SYSTEM_EVENT_DOMAIN_FAULT;
    capture.state = state;
    capture.code = (uint16_t)code;
    capture.detail = detail;
    capture.configuration_revision = configuration_revision;
    capture.correlation_event_id = correlation_event_id;
    (void)SystemEventService_Record(&capture, NULL);
}

static void RecordSafetyResetEvent(uint32_t safety_source_mask,
                                   uint32_t timestamp_ms,
                                   uint32_t correlation_event_id)
{
    SystemEventCapture_t capture;

    (void)memset(&capture, 0, sizeof(capture));
    capture.timestamp_ms = timestamp_ms;
    capture.domain = SYSTEM_EVENT_DOMAIN_SAFETY;
    capture.state = SYSTEM_EVENT_STATE_RESET;
    capture.code = (uint16_t)safety_source_mask;
    capture.correlation_event_id = correlation_event_id;
    (void)SystemEventService_Record(&capture, NULL);
}

static void CaptureFault(FaultCode_t code,
                         uint16_t detail,
                         uint16_t configuration_revision,
                         uint32_t correlation_event_id,
                         uint32_t timestamp_ms,
                         const int32_t values[SNAPSHOT_SERVICE_VALUE_COUNT])
{
    SnapshotCapture_t capture;

    (void)memset(&capture, 0, sizeof(capture));
    capture.timestamp_ms = timestamp_ms;
    capture.source = SNAPSHOT_SOURCE_FAULT;
    capture.code = (uint16_t)code;
    capture.detail = detail;
    capture.configuration_revision = configuration_revision;
    capture.event_id = correlation_event_id;
    if (values != NULL)
    {
        (void)memcpy(capture.values, values, sizeof(capture.values));
    }
    (void)SnapshotService_Capture(&capture, NULL);
}

bool SystemFaultService_Raise(
    FaultCode_t code,
    uint16_t detail,
    uint16_t configuration_revision,
    uint32_t correlation_event_id,
    uint32_t timestamp_ms,
    const int32_t values[SNAPSHOT_SERVICE_VALUE_COUNT])
{
    bool was_active = FaultService_IsActive(code);

    if (!FaultService_Raise(code, detail, configuration_revision,
                            correlation_event_id))
    {
        return false;
    }
    if (!was_active)
    {
        RecordFaultEvent(code, SYSTEM_EVENT_STATE_ASSERTED, detail,
                         configuration_revision, correlation_event_id,
                         timestamp_ms);
        CaptureFault(code, detail, configuration_revision,
                     correlation_event_id, timestamp_ms, values);
    }
    return true;
}

bool SystemFaultService_ClearRecovered(
    FaultCode_t code,
    uint32_t timestamp_ms,
    uint32_t correlation_event_id)
{
    FaultRecord_t record;

    if (!FaultService_Get(code, &record))
    {
        return false;
    }
    if (!record.active)
    {
        return true;
    }
    if (!FaultService_Clear(code))
    {
        return false;
    }
    RecordFaultEvent(code, SYSTEM_EVENT_STATE_CLEARED,
                     record.last_detail,
                     record.last_configuration_revision,
                     correlation_event_id, timestamp_ms);
    return true;
}

SystemFaultResetResult_t SystemFaultService_Reset(
    FaultCode_t code,
    uint32_t safety_source_mask,
    uint32_t timestamp_ms,
    uint32_t correlation_event_id)
{
    FaultRecord_t record;

    if (!FaultService_Get(code, &record))
    {
        return SYSTEM_FAULT_RESET_INVALID_ARGUMENT;
    }
    if (!record.active)
    {
        return SYSTEM_FAULT_RESET_NOT_ACTIVE;
    }
    if (safety_source_mask != 0U)
    {
        SafetyServiceStatus_t safety_status;
        SafetyResetResult_t reset_result;

        if (!SafetyService_GetStatus(&safety_status) ||
            ((safety_source_mask & ~SAFETY_SOURCE_ALL) != 0U))
        {
            return SYSTEM_FAULT_RESET_INVALID_ARGUMENT;
        }
        if ((safety_status.active_source_mask & safety_source_mask) != 0U)
        {
            return SYSTEM_FAULT_RESET_CONDITION_ACTIVE;
        }
        reset_result = SafetyService_Reset(safety_source_mask);
        if (reset_result != SAFETY_RESET_OK)
        {
            return (reset_result == SAFETY_RESET_BLOCKED_ACTIVE) ?
                SYSTEM_FAULT_RESET_CONDITION_ACTIVE :
                SYSTEM_FAULT_RESET_FAILED;
        }
    }
    if (!FaultService_Clear(code))
    {
        return SYSTEM_FAULT_RESET_FAILED;
    }

    RecordFaultEvent(code, SYSTEM_EVENT_STATE_RESET,
                     record.last_detail,
                     record.last_configuration_revision,
                     correlation_event_id, timestamp_ms);
    if (safety_source_mask != 0U)
    {
        RecordSafetyResetEvent(safety_source_mask, timestamp_ms,
                               correlation_event_id);
    }
    return SYSTEM_FAULT_RESET_OK;
}
