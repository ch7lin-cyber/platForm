#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "SystemEventService.h"

static SystemEventCapture_t Capture(uint32_t timestamp_ms,
                                    uint16_t code)
{
    SystemEventCapture_t capture = {0};

    capture.timestamp_ms = timestamp_ms;
    capture.domain = SYSTEM_EVENT_DOMAIN_WARNING;
    capture.state = SYSTEM_EVENT_STATE_ASSERTED;
    capture.code = code;
    capture.detail = (uint16_t)(code + 1U);
    capture.configuration_revision = 3U;
    capture.correlation_event_id = 9U;
    return capture;
}

static void TestRecordAndOrder(void)
{
    SystemEventCapture_t capture;
    SystemEventRecord_t record;
    uint32_t first_sequence;

    SystemEventService_Initialize();
    capture = Capture(100U, 10U);
    assert(SystemEventService_Record(&capture, &first_sequence));
    capture = Capture(200U, 20U);
    capture.state = SYSTEM_EVENT_STATE_CLEARED;
    assert(SystemEventService_Record(&capture, NULL));
    assert(SystemEventService_GetCount() == 2U);

    assert(SystemEventService_GetOldest(0U, &record));
    assert(record.sequence == first_sequence);
    assert(record.timestamp_ms == 100U);
    assert(record.code == 10U);
    assert(record.detail == 11U);
    assert(record.configuration_revision == 3U);
    assert(record.correlation_event_id == 9U);

    assert(SystemEventService_GetNewest(0U, &record));
    assert(record.timestamp_ms == 200U);
    assert(record.state == SYSTEM_EVENT_STATE_CLEARED);
    assert(SystemEventService_GetBySequence(first_sequence, &record));
}

static void TestRingOverwriteAndClear(void)
{
    SystemEventCapture_t capture;
    SystemEventRecord_t record;
    uint16_t index;
    uint32_t sequence_before_clear;

    SystemEventService_Initialize();
    for (index = 0U; index < (SYSTEM_EVENT_SERVICE_CAPACITY + 3U); index++)
    {
        capture = Capture(index, index);
        assert(SystemEventService_Record(&capture, NULL));
    }
    assert(SystemEventService_GetCount() == SYSTEM_EVENT_SERVICE_CAPACITY);
    assert(SystemEventService_GetOldest(0U, &record));
    assert(record.timestamp_ms == 3U);
    assert(SystemEventService_GetNewest(0U, &record));
    assert(record.timestamp_ms == SYSTEM_EVENT_SERVICE_CAPACITY + 2U);
    sequence_before_clear = record.sequence;

    SystemEventService_Clear();
    assert(SystemEventService_GetCount() == 0U);
    capture = Capture(500U, 5U);
    assert(SystemEventService_Record(&capture, NULL));
    assert(SystemEventService_GetNewest(0U, &record));
    assert(record.sequence > sequence_before_clear);
}

static void TestValidation(void)
{
    SystemEventCapture_t capture = {0};
    SystemEventRecord_t record;

    SystemEventService_Initialize();
    assert(!SystemEventService_Record(NULL, NULL));
    assert(!SystemEventService_Record(&capture, NULL));
    assert(!SystemEventService_GetNewest(0U, &record));
    assert(!SystemEventService_GetOldest(0U, &record));
    assert(!SystemEventService_GetBySequence(0U, &record));
}

int main(void)
{
    TestRecordAndOrder();
    TestRingOverwriteAndClear();
    TestValidation();
    return 0;
}
