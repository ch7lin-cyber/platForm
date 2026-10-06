#ifndef SNAPSHOT_SERVICE_H
#define SNAPSHOT_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SNAPSHOT_SERVICE_CAPACITY    (16U)
#define SNAPSHOT_SERVICE_VALUE_COUNT (8U)

typedef enum
{
    SNAPSHOT_SOURCE_NONE = 0,
    SNAPSHOT_SOURCE_EVENT,
    SNAPSHOT_SOURCE_WARNING,
    SNAPSHOT_SOURCE_SAFETY,
    SNAPSHOT_SOURCE_FAULT,
    SNAPSHOT_SOURCE_APPLICATION
} SnapshotSource_t;

typedef struct
{
    uint32_t timestamp_ms;
    SnapshotSource_t source;
    uint16_t code;
    uint16_t detail;
    uint16_t configuration_revision;
    uint32_t event_id;
    int32_t values[SNAPSHOT_SERVICE_VALUE_COUNT];
} SnapshotCapture_t;

typedef struct
{
    uint32_t sequence;
    uint32_t timestamp_ms;
    SnapshotSource_t source;
    uint16_t code;
    uint16_t detail;
    uint16_t configuration_revision;
    uint32_t event_id;
    int32_t values[SNAPSHOT_SERVICE_VALUE_COUNT];
} SnapshotRecord_t;

void SnapshotService_Initialize(void);

bool SnapshotService_Capture(const SnapshotCapture_t *capture,
                             uint32_t *sequence);

uint16_t SnapshotService_GetCount(void);

/* age 0 is the newest record; age count-1 is the oldest retained record. */
bool SnapshotService_GetNewest(uint16_t age, SnapshotRecord_t *record);

/* index 0 is the oldest retained record. */
bool SnapshotService_GetOldest(uint16_t index, SnapshotRecord_t *record);

bool SnapshotService_GetBySequence(uint32_t sequence,
                                   SnapshotRecord_t *record);

void SnapshotService_Clear(void);

#ifdef __cplusplus
}
#endif

#endif /* SNAPSHOT_SERVICE_H */
