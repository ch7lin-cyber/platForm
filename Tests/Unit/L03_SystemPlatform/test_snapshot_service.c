#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "SnapshotService.h"

static SnapshotCapture_t BuildCapture(uint32_t value)
{
    SnapshotCapture_t capture = {0};

    capture.timestamp_ms = value * 10U;
    capture.source = SNAPSHOT_SOURCE_FAULT;
    capture.code = (uint16_t)(0x0100U + value);
    capture.detail = (uint16_t)value;
    capture.configuration_revision = (uint16_t)(value + 1U);
    capture.event_id = value + 100U;
    capture.values[0] = (int32_t)value;
    capture.values[1] = -(int32_t)value;
    return capture;
}

static void TestCaptureAndQuery(void)
{
    SnapshotCapture_t first = BuildCapture(1U);
    SnapshotCapture_t second = BuildCapture(2U);
    SnapshotRecord_t record;
    uint32_t first_sequence;
    uint32_t second_sequence;

    SnapshotService_Initialize();
    assert(SnapshotService_GetCount() == 0U);
    assert(SnapshotService_Capture(&first, &first_sequence));
    assert(SnapshotService_Capture(&second, &second_sequence));
    assert(first_sequence == 1U);
    assert(second_sequence == 2U);
    assert(SnapshotService_GetCount() == 2U);

    assert(SnapshotService_GetNewest(0U, &record));
    assert(record.sequence == second_sequence);
    assert(record.timestamp_ms == 20U);
    assert(record.code == 0x0102U);
    assert(record.values[0] == 2);
    assert(record.values[1] == -2);

    assert(SnapshotService_GetOldest(0U, &record));
    assert(record.sequence == first_sequence);
    assert(SnapshotService_GetBySequence(second_sequence, &record));
    assert(record.event_id == 102U);
}

static void TestRingBufferRetainsNewestRecords(void)
{
    SnapshotRecord_t record;
    uint32_t value;

    SnapshotService_Initialize();
    for (value = 1U; value <= (SNAPSHOT_SERVICE_CAPACITY + 4U); value++)
    {
        SnapshotCapture_t capture = BuildCapture(value);
        assert(SnapshotService_Capture(&capture, NULL));
    }

    assert(SnapshotService_GetCount() == SNAPSHOT_SERVICE_CAPACITY);
    assert(SnapshotService_GetOldest(0U, &record));
    assert(record.values[0] == 5);
    assert(SnapshotService_GetNewest(0U, &record));
    assert(record.values[0] ==
           (int32_t)(SNAPSHOT_SERVICE_CAPACITY + 4U));
    assert(!SnapshotService_GetBySequence(1U, &record));
}

static void TestValidationAndClear(void)
{
    SnapshotCapture_t capture = BuildCapture(1U);
    SnapshotRecord_t record;

    SnapshotService_Initialize();
    assert(!SnapshotService_Capture(NULL, NULL));
    capture.source = SNAPSHOT_SOURCE_NONE;
    assert(!SnapshotService_Capture(&capture, NULL));
    capture.source = (SnapshotSource_t)99;
    assert(!SnapshotService_Capture(&capture, NULL));
    assert(!SnapshotService_GetNewest(0U, &record));
    assert(!SnapshotService_GetOldest(0U, &record));
    assert(!SnapshotService_GetBySequence(0U, &record));

    capture = BuildCapture(3U);
    assert(SnapshotService_Capture(&capture, NULL));
    SnapshotService_Clear();
    assert(SnapshotService_GetCount() == 0U);
    assert(!SnapshotService_GetNewest(0U, &record));
}

int main(void)
{
    TestCaptureAndQuery();
    TestRingBufferRetainsNewestRecords();
    TestValidationAndClear();
    return 0;
}
