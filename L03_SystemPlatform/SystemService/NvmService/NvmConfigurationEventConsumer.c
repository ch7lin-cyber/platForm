#include "NvmConfigurationEventConsumer.h"

#include "EventService.h"
#include "FaultService.h"
#include "NvmService.h"

static uint32_t g_queued_event_id;
static uint16_t g_queued_revision;
static FaultCode_t g_fault_code;
static bool g_fault_raised;
static bool g_service_available;

static FaultCode_t MapError(NvmServiceError_t error)
{
    switch (error)
    {
        case NVM_SERVICE_ERROR_ERASE:
            return FAULT_CODE_NVM_ERASE_FAILED;
        case NVM_SERVICE_ERROR_WRITE_DATA:
            return FAULT_CODE_NVM_DATA_PROGRAM_FAILED;
        case NVM_SERVICE_ERROR_WRITE_COMMIT:
            return FAULT_CODE_NVM_COMMIT_PROGRAM_FAILED;
        case NVM_SERVICE_ERROR_VERIFY:
            return FAULT_CODE_NVM_VERIFY_FAILED;
        default:
            return FAULT_CODE_NVM_INITIALIZATION_FAILED;
    }
}

static void RaiseFault(FaultCode_t code, uint16_t detail,
                       uint16_t revision, uint32_t event_id)
{
    if (!g_fault_raised &&
        FaultService_Raise(code, detail, revision, event_id))
    {
        g_fault_code = code;
        g_fault_raised = true;
    }
}

bool NvmConfigurationEventConsumer_Initialize(void)
{
    g_queued_event_id = 0U;
    g_queued_revision = 0U;
    g_fault_code = FAULT_CODE_NONE;
    g_fault_raised = false;
    g_service_available = NvmService_Initialize();
    if (!g_service_available)
    {
        RaiseFault(FAULT_CODE_NVM_INITIALIZATION_FAILED,
                   (uint16_t)NvmService_GetLastError(), 0U, 0U);
    }
    /* Initialization fault is reported; keep communication/HMI alive. */
    return true;
}

bool NvmConfigurationEventConsumer_Process(uint8_t channel)
{
    TemperatureInputConfigurationChangedEvent_t event;
    EventTemperatureInputConfiguration_t loaded_configuration;
    uint16_t loaded_revision;

    if (!g_service_available)
    {
        if (g_fault_raised && FaultService_IsActive(g_fault_code))
        {
            return false;
        }
        g_fault_raised = false;
        g_fault_code = FAULT_CODE_NONE;
        g_service_available = NvmService_Initialize();
        if (!g_service_available)
        {
            RaiseFault(FAULT_CODE_NVM_INITIALIZATION_FAILED,
                       (uint16_t)NvmService_GetLastError(), 0U, 0U);
        }
        return g_service_available;
    }
    NvmServiceState_t state;

    if (!EventService_GetTemperatureInputConfigurationChanged(channel, &event))
    {
        return true;
    }

    if (g_fault_raised)
    {
        if (FaultService_IsActive(g_fault_code))
        {
            return false;
        }
        g_fault_raised = false;
        g_fault_code = FAULT_CODE_NONE;
        if (NvmService_GetState() == NVM_SERVICE_STATE_ERROR)
        {
            NvmService_ResetError();
            g_queued_event_id = 0U;
            g_queued_revision = 0U;
            return false;
        }
    }

    if (g_queued_event_id == 0U)
    {
        if (NvmService_GetLoadedTemperatureInputConfiguration(
                &loaded_revision, &loaded_configuration) &&
            (loaded_revision == event.configuration_revision) &&
            (loaded_configuration.filter_time_constant_seconds ==
             event.new_configuration.filter_time_constant_seconds) &&
            (loaded_configuration.sensor_type ==
             event.new_configuration.sensor_type) &&
            (loaded_configuration.tc_linearization ==
             event.new_configuration.tc_linearization))
        {
            return EventService_Acknowledge(event.event_id, EVENT_ACK_NVM);
        }
        if (!NvmService_QueueTemperatureInputConfiguration(
                event.configuration_revision, &event.new_configuration))
        {
            return false;
        }
        g_queued_event_id = event.event_id;
        g_queued_revision = event.configuration_revision;
    }
    else if (g_queued_event_id != event.event_id)
    {
        return false;
    }

    NvmService_Process();
    state = NvmService_GetState();
    if (state == NVM_SERVICE_STATE_ERROR)
    {
        FaultCode_t code = MapError(NvmService_GetLastError());
        RaiseFault(code, (uint16_t)NvmService_GetLastError(),
                   event.configuration_revision, event.event_id);
        return false;
    }
    if (state == NVM_SERVICE_STATE_COMPLETE)
    {
        if (NvmService_GetCompletedRevision() != g_queued_revision)
        {
            return false;
        }
        if (!EventService_Acknowledge(g_queued_event_id, EVENT_ACK_NVM))
        {
            RaiseFault(FAULT_CODE_NVM_EVENT_ACK_FAILED, 0U,
                       g_queued_revision, g_queued_event_id);
            return false;
        }
        NvmService_AcknowledgeCompletion();
        g_queued_event_id = 0U;
        g_queued_revision = 0U;
    }
    return true;
}
