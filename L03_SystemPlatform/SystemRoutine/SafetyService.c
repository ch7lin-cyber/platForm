#include "SafetyService.h"

#include <stddef.h>
#include <string.h>

typedef struct
{
    uint32_t active_source_mask;
    uint32_t latched_source_mask;
    uint32_t latching_source_mask;
    uint32_t transition_count;
    SafetyState_t state;
    SafetyOutputAction_t output_action;
    void *output_action_context;
    bool output_inhibited;
    bool initialized;
} SafetyServiceContext_t;

static SafetyServiceContext_t g_safety;

static uint32_t TripSourceMask(void)
{
    return g_safety.active_source_mask | g_safety.latched_source_mask;
}

static bool CaptureAssertion(
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
    capture.source = SNAPSHOT_SOURCE_SAFETY;
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

bool SafetyService_Initialize(uint32_t latching_source_mask,
                              SafetyOutputAction_t output_action,
                              void *output_action_context)
{
    if ((latching_source_mask & ~SAFETY_SOURCE_ALL) != 0U)
    {
        return false;
    }

    (void)memset(&g_safety, 0, sizeof(g_safety));
    g_safety.latching_source_mask = latching_source_mask;
    g_safety.output_action = output_action;
    g_safety.output_action_context = output_action_context;
    g_safety.state = SAFETY_STATE_NORMAL;
    g_safety.initialized = true;
    return true;
}

bool SafetyService_UpdateSource(
    uint32_t source_mask,
    bool active,
    uint16_t detail,
    uint32_t timestamp_ms,
    uint16_t configuration_revision,
    uint32_t event_id,
    const int32_t values[SNAPSHOT_SERVICE_VALUE_COUNT])
{
    uint32_t newly_active;

    if (!g_safety.initialized || (source_mask == 0U) ||
        ((source_mask & ~SAFETY_SOURCE_ALL) != 0U))
    {
        return false;
    }

    newly_active = active ?
        (source_mask & ~g_safety.active_source_mask) : 0U;
    if (active)
    {
        g_safety.active_source_mask |= source_mask;
        g_safety.latched_source_mask |=
            source_mask & g_safety.latching_source_mask;
    }
    else
    {
        g_safety.active_source_mask &= ~source_mask;
    }

    if ((newly_active != 0U) &&
        !CaptureAssertion(newly_active, detail, timestamp_ms,
                          configuration_revision, event_id, values))
    {
        return false;
    }
    return true;
}

bool SafetyService_Process(void)
{
    uint32_t trip_source_mask;
    bool inhibit_required;

    if (!g_safety.initialized)
    {
        return false;
    }

    trip_source_mask = TripSourceMask();
    inhibit_required = trip_source_mask != 0U;
    if (inhibit_required != g_safety.output_inhibited)
    {
        if ((g_safety.output_action != NULL) &&
            !g_safety.output_action(inhibit_required, trip_source_mask,
                                    g_safety.output_action_context))
        {
            g_safety.state = SAFETY_STATE_ACTION_ERROR;
            return false;
        }
        g_safety.output_inhibited = inhibit_required;
        g_safety.transition_count++;
    }

    g_safety.state = inhibit_required ? SAFETY_STATE_TRIPPED :
                                        SAFETY_STATE_NORMAL;
    return true;
}

SafetyResetResult_t SafetyService_Reset(uint32_t source_mask)
{
    if (!g_safety.initialized || (source_mask == 0U) ||
        ((source_mask & ~SAFETY_SOURCE_ALL) != 0U))
    {
        return SAFETY_RESET_INVALID_ARGUMENT;
    }
    if ((g_safety.active_source_mask & source_mask) != 0U)
    {
        return SAFETY_RESET_BLOCKED_ACTIVE;
    }

    g_safety.latched_source_mask &= ~source_mask;
    return SAFETY_RESET_OK;
}

bool SafetyService_GetStatus(SafetyServiceStatus_t *status)
{
    if (!g_safety.initialized || (status == NULL))
    {
        return false;
    }

    status->active_source_mask = g_safety.active_source_mask;
    status->latched_source_mask = g_safety.latched_source_mask;
    status->trip_source_mask = TripSourceMask();
    status->latching_source_mask = g_safety.latching_source_mask;
    status->transition_count = g_safety.transition_count;
    status->state = g_safety.state;
    status->output_inhibited = g_safety.output_inhibited;
    return true;
}

bool SafetyService_IsOutputInhibited(void)
{
    return g_safety.initialized && g_safety.output_inhibited;
}
