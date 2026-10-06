#include "SnapshotService.h"

#include <stddef.h>
#include <string.h>

static SnapshotRecord_t g_records[SNAPSHOT_SERVICE_CAPACITY];
static uint16_t g_write_index;
static uint16_t g_record_count;
static uint32_t g_next_sequence;
static bool g_initialized;

static uint16_t OldestPhysicalIndex(void)
{
    if (g_record_count < SNAPSHOT_SERVICE_CAPACITY)
    {
        return 0U;
    }
    return g_write_index;
}

static uint16_t PhysicalIndexFromOldest(uint16_t logical_index)
{
    return (uint16_t)((OldestPhysicalIndex() + logical_index) %
                      SNAPSHOT_SERVICE_CAPACITY);
}

void SnapshotService_Initialize(void)
{
    (void)memset(g_records, 0, sizeof(g_records));
    g_write_index = 0U;
    g_record_count = 0U;
    g_next_sequence = 0U;
    g_initialized = true;
}

bool SnapshotService_Capture(const SnapshotCapture_t *capture,
                             uint32_t *sequence)
{
    SnapshotRecord_t *record;

    if (!g_initialized || (capture == NULL) ||
        (capture->source <= SNAPSHOT_SOURCE_NONE) ||
        (capture->source > SNAPSHOT_SOURCE_APPLICATION))
    {
        return false;
    }

    g_next_sequence++;
    if (g_next_sequence == 0U)
    {
        g_next_sequence = 1U;
    }

    record = &g_records[g_write_index];
    record->sequence = g_next_sequence;
    record->timestamp_ms = capture->timestamp_ms;
    record->source = capture->source;
    record->code = capture->code;
    record->detail = capture->detail;
    record->configuration_revision = capture->configuration_revision;
    record->event_id = capture->event_id;
    (void)memcpy(record->values, capture->values,
                 sizeof(record->values));

    g_write_index++;
    if (g_write_index >= SNAPSHOT_SERVICE_CAPACITY)
    {
        g_write_index = 0U;
    }
    if (g_record_count < SNAPSHOT_SERVICE_CAPACITY)
    {
        g_record_count++;
    }
    if (sequence != NULL)
    {
        *sequence = record->sequence;
    }
    return true;
}

uint16_t SnapshotService_GetCount(void)
{
    return g_initialized ? g_record_count : 0U;
}

bool SnapshotService_GetNewest(uint16_t age, SnapshotRecord_t *record)
{
    uint16_t logical_index;

    if (!g_initialized || (record == NULL) || (age >= g_record_count))
    {
        return false;
    }
    logical_index = (uint16_t)(g_record_count - 1U - age);
    *record = g_records[PhysicalIndexFromOldest(logical_index)];
    return true;
}

bool SnapshotService_GetOldest(uint16_t index, SnapshotRecord_t *record)
{
    if (!g_initialized || (record == NULL) || (index >= g_record_count))
    {
        return false;
    }
    *record = g_records[PhysicalIndexFromOldest(index)];
    return true;
}

bool SnapshotService_GetBySequence(uint32_t sequence,
                                   SnapshotRecord_t *record)
{
    uint16_t index;

    if (!g_initialized || (sequence == 0U) || (record == NULL))
    {
        return false;
    }
    for (index = 0U; index < g_record_count; index++)
    {
        SnapshotRecord_t *candidate =
            &g_records[PhysicalIndexFromOldest(index)];
        if (candidate->sequence == sequence)
        {
            *record = *candidate;
            return true;
        }
    }
    return false;
}

void SnapshotService_Clear(void)
{
    if (!g_initialized)
    {
        return;
    }
    (void)memset(g_records, 0, sizeof(g_records));
    g_write_index = 0U;
    g_record_count = 0U;
}
