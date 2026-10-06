#include "WarningService.h"

#include <stddef.h>
#include <string.h>

#include "SystemEventService.h"

#define WARNING_SERVICE_SOURCE_COUNT (11U)

typedef struct
{
    uint32_t source_mask;
    WarningSourceConfiguration_t configuration;
    uint16_t assert_counter;
    uint16_t clear_counter;
} WarningSourceState_t;

static WarningSourceState_t g_sources[WARNING_SERVICE_SOURCE_COUNT];
static WarningServiceStatus_t g_status;
static bool g_initialized;

static int16_t FindSource(uint32_t source_mask)
{
    uint16_t index;

    for (index = 0U; index < WARNING_SERVICE_SOURCE_COUNT; index++)
    {
        if (g_sources[index].source_mask == source_mask)
        {
            return (int16_t)index;
        }
    }
    return -1;
}

static bool CaptureWarning(
    uint32_t source_mask,
    uint16_t detail,
    uint32_t timestamp_ms,
    uint16_t configuration_revision,
    uint32_t event_id,
    const int32_t values[SNAPSHOT_SERVICE_VALUE_COUNT])
{
    SnapshotCapture_t capture;

    (void)memset(&capture, 0, sizeof(capture));
    capture.timestamp_ms = timestamp_ms;
    capture.source = SNAPSHOT_SOURCE_WARNING;
    capture.code = (uint16_t)source_mask;
    capture.detail = detail;
    capture.configuration_revision = configuration_revision;
    capture.event_id = event_id;
    if (values != NULL)
    {
        (void)memcpy(capture.values, values, sizeof(capture.values));
    }
    return SnapshotService_Capture(&capture, NULL);
}

static void RecordWarningEvent(
    uint32_t source_mask,
    SystemEventState_t state,
    uint16_t detail,
    uint32_t timestamp_ms,
    uint16_t configuration_revision,
    uint32_t event_id)
{
    SystemEventCapture_t capture;

    (void)memset(&capture, 0, sizeof(capture));
    capture.timestamp_ms = timestamp_ms;
    capture.domain = SYSTEM_EVENT_DOMAIN_WARNING;
    capture.state = state;
    capture.code = (uint16_t)source_mask;
    capture.detail = detail;
    capture.configuration_revision = configuration_revision;
    capture.correlation_event_id = event_id;
    (void)SystemEventService_Record(&capture, NULL);
}

void WarningService_Initialize(void)
{
    static const uint32_t source_masks[WARNING_SERVICE_SOURCE_COUNT] =
    {
        WARNING_SOURCE_ADC_COMMUNICATION,
        WARNING_SOURCE_ADC_INTEGRITY,
        WARNING_SOURCE_ADC_REFERENCE,
        WARNING_SOURCE_ADC_CONVERSION,
        WARNING_SOURCE_ADC_INPUT_VOLTAGE,
        WARNING_SOURCE_ADC_STALE,
        WARNING_SOURCE_LOW_VOLTAGE,
        WARNING_SOURCE_MCU_TEMPERATURE,
        WARNING_SOURCE_NVM,
        WARNING_SOURCE_ADC_INTERNAL,
        WARNING_SOURCE_APPLICATION
    };
    uint16_t index;

    (void)memset(g_sources, 0, sizeof(g_sources));
    (void)memset(&g_status, 0, sizeof(g_status));
    for (index = 0U; index < WARNING_SERVICE_SOURCE_COUNT; index++)
    {
        g_sources[index].source_mask = source_masks[index];
        g_sources[index].configuration.assert_count = 1U;
        g_sources[index].configuration.clear_count = 1U;
    }
    g_initialized = true;
}

bool WarningService_ConfigureSource(
    uint32_t source_mask,
    const WarningSourceConfiguration_t *configuration)
{
    int16_t index;

    if ((!g_initialized) || (configuration == NULL) ||
        (configuration->assert_count == 0U) ||
        (configuration->clear_count == 0U))
    {
        return false;
    }
    index = FindSource(source_mask);
    if (index < 0)
    {
        return false;
    }
    if (((g_status.active_source_mask | g_status.latched_source_mask) &
         source_mask) != 0U)
    {
        return false;
    }

    g_sources[(uint16_t)index].configuration = *configuration;
    g_sources[(uint16_t)index].assert_counter = 0U;
    g_sources[(uint16_t)index].clear_counter = 0U;
    return true;
}

bool WarningService_UpdateSource(
    uint32_t source_mask,
    bool condition_present,
    uint16_t detail,
    uint32_t timestamp_ms,
    uint16_t configuration_revision,
    uint32_t event_id,
    const int32_t values[SNAPSHOT_SERVICE_VALUE_COUNT])
{
    WarningSourceState_t *source;
    int16_t index;
    bool active;

    if (!g_initialized)
    {
        return false;
    }
    index = FindSource(source_mask);
    if (index < 0)
    {
        return false;
    }
    source = &g_sources[(uint16_t)index];
    active = (g_status.active_source_mask & source_mask) != 0U;

    if (condition_present)
    {
        source->clear_counter = 0U;
        if (active)
        {
            source->assert_counter = 0U;
            return true;
        }
        if (source->assert_counter < source->configuration.assert_count)
        {
            source->assert_counter++;
        }
        if (source->assert_counter < source->configuration.assert_count)
        {
            return true;
        }

        source->assert_counter = 0U;
        g_status.active_source_mask |= source_mask;
        if (source->configuration.latching)
        {
            g_status.latched_source_mask |= source_mask;
        }
        g_status.transition_count++;
        g_status.warning_source_mask =
            g_status.active_source_mask | g_status.latched_source_mask;

        RecordWarningEvent(source_mask, SYSTEM_EVENT_STATE_ASSERTED,
                           detail, timestamp_ms,
                           configuration_revision, event_id);

        /* Warning state remains asserted even if diagnostic capture fails. */
        return CaptureWarning(source_mask, detail, timestamp_ms,
                              configuration_revision, event_id, values);
    }

    source->assert_counter = 0U;
    if (!active)
    {
        source->clear_counter = 0U;
        return true;
    }
    if (source->clear_counter < source->configuration.clear_count)
    {
        source->clear_counter++;
    }
    if (source->clear_counter < source->configuration.clear_count)
    {
        return true;
    }

    source->clear_counter = 0U;
    g_status.active_source_mask &= ~source_mask;
    g_status.transition_count++;
    g_status.warning_source_mask =
        g_status.active_source_mask | g_status.latched_source_mask;
    RecordWarningEvent(source_mask, SYSTEM_EVENT_STATE_CLEARED,
                       detail, timestamp_ms,
                       configuration_revision, event_id);
    return true;
}

bool WarningService_Reset(uint32_t source_mask)
{
    if ((!g_initialized) || (source_mask == 0U) ||
        ((source_mask & ~WARNING_SOURCE_ALL) != 0U) ||
        ((g_status.active_source_mask & source_mask) != 0U))
    {
        return false;
    }

    g_status.latched_source_mask &= ~source_mask;
    g_status.warning_source_mask =
        g_status.active_source_mask | g_status.latched_source_mask;
    return true;
}

bool WarningService_GetStatus(WarningServiceStatus_t *status)
{
    if ((!g_initialized) || (status == NULL))
    {
        return false;
    }
    *status = g_status;
    return true;
}

bool WarningService_IsActive(uint32_t source_mask)
{
    if ((!g_initialized) || (source_mask == 0U) ||
        ((source_mask & ~WARNING_SOURCE_ALL) != 0U))
    {
        return false;
    }
    return (g_status.warning_source_mask & source_mask) != 0U;
}
