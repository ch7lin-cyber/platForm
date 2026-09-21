#include "NvmConfigurationEventConsumer.h"

#include "EventService.h"
#include "NvmService.h"

static uint32_t g_queued_event_id;
static uint16_t g_queued_revision;

bool NvmConfigurationEventConsumer_Initialize(void)
{
    g_queued_event_id = 0U;
    g_queued_revision = 0U;
    return NvmService_Initialize();
}

bool NvmConfigurationEventConsumer_Process(uint8_t channel)
{
    TemperatureInputConfigurationChangedEvent_t event;
    EventTemperatureInputConfiguration_t loaded_configuration;
    uint16_t loaded_revision;
    NvmServiceState_t state;

    if (!EventService_GetTemperatureInputConfigurationChanged(channel, &event))
    {
        return true;
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
            return false;
        }
        NvmService_AcknowledgeCompletion();
        g_queued_event_id = 0U;
        g_queued_revision = 0U;
    }
    return true;
}
