#include "SystemEventService.h"

#include <stddef.h>
#include <string.h>

static SystemEventRecord_t
    g_records[SYSTEM_EVENT_SERVICE_CAPACITY];
static uint32_t g_next_sequence;
static uint16_t g_write_index;
static uint16_t g_count;
static bool g_initialized;

static uint16_t OldestPhysicalIndex(void)
{
    return (uint16_t)((g_write_index + SYSTEM_EVENT_SERVICE_CAPACITY -
                       g_count) % SYSTEM_EVENT_SERVICE_CAPACITY);
}

void SystemEventService_Initialize(void)
{
    (void)memset(g_records, 0, sizeof(g_records));
    g_next_sequence = 0U;
    g_write_index = 0U;
    g_count = 0U;
    g_initialized = true;
}

bool SystemEventService_Record(const SystemEventCapture_t *capture,
                               uint32_t *sequence)
{
    SystemEventRecord_t *record;

    if ((!g_initialized) || (capture == NULL) ||
        (capture->domain == SYSTEM_EVENT_DOMAIN_NONE) ||
        (capture->state == SYSTEM_EVENT_STATE_NONE))
    {
        return false;
    }

    g_next_sequence++;
    if (g_next_sequence == 0U)
    {
        g_next_sequence++;
    }

    record = &g_records[g_write_index];
    record->sequence = g_next_sequence;
    record->timestamp_ms = capture->timestamp_ms;
    record->domain = capture->domain;
    record->state = capture->state;
    record->code = capture->code;
    record->detail = capture->detail;
    record->configuration_revision =
        capture->configuration_revision;
    record->correlation_event_id = capture->correlation_event_id;

    g_write_index = (uint16_t)((g_write_index + 1U) %
                               SYSTEM_EVENT_SERVICE_CAPACITY);
    if (g_count < SYSTEM_EVENT_SERVICE_CAPACITY)
    {
        g_count++;
    }
    if (sequence != NULL)
    {
        *sequence = record->sequence;
    }
    return true;
}

uint16_t SystemEventService_GetCount(void)
{
    return g_initialized ? g_count : 0U;
}

bool SystemEventService_GetOldest(uint16_t index,
                                  SystemEventRecord_t *record)
{
    uint16_t physical_index;

    if ((!g_initialized) || (record == NULL) || (index >= g_count))
    {
        return false;
    }
    physical_index = (uint16_t)((OldestPhysicalIndex() + index) %
                                SYSTEM_EVENT_SERVICE_CAPACITY);
    *record = g_records[physical_index];
    return true;
}

bool SystemEventService_GetNewest(uint16_t age,
                                  SystemEventRecord_t *record)
{
    if (age >= g_count)
    {
        return false;
    }
    return SystemEventService_GetOldest(
        (uint16_t)(g_count - 1U - age), record);
}

bool SystemEventService_GetBySequence(uint32_t sequence,
                                      SystemEventRecord_t *record)
{
    uint16_t index;
    SystemEventRecord_t candidate;

    if ((!g_initialized) || (sequence == 0U) || (record == NULL))
    {
        return false;
    }
    for (index = 0U; index < g_count; index++)
    {
        if (SystemEventService_GetOldest(index, &candidate) &&
            (candidate.sequence == sequence))
        {
            *record = candidate;
            return true;
        }
    }
    return false;
}

void SystemEventService_Clear(void)
{
    if (g_initialized)
    {
        (void)memset(g_records, 0, sizeof(g_records));
        g_write_index = 0U;
        g_count = 0U;
    }
}
