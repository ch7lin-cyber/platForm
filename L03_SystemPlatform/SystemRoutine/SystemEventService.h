#ifndef SYSTEM_EVENT_SERVICE_H
#define SYSTEM_EVENT_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYSTEM_EVENT_SERVICE_CAPACITY (32U)

typedef enum
{
    SYSTEM_EVENT_DOMAIN_NONE = 0,
    SYSTEM_EVENT_DOMAIN_WARNING,
    SYSTEM_EVENT_DOMAIN_SAFETY,
    SYSTEM_EVENT_DOMAIN_FAULT,
    SYSTEM_EVENT_DOMAIN_SYSTEM
} SystemEventDomain_t;

typedef enum
{
    SYSTEM_EVENT_STATE_NONE = 0,
    SYSTEM_EVENT_STATE_ASSERTED,
    SYSTEM_EVENT_STATE_CLEARED,
    SYSTEM_EVENT_STATE_RESET,
    SYSTEM_EVENT_STATE_ACTION_ERROR
} SystemEventState_t;

typedef struct
{
    uint32_t timestamp_ms;
    SystemEventDomain_t domain;
    SystemEventState_t state;
    uint16_t code;
    uint16_t detail;
    uint16_t configuration_revision;
    uint32_t correlation_event_id;
} SystemEventCapture_t;

typedef struct
{
    uint32_t sequence;
    uint32_t timestamp_ms;
    SystemEventDomain_t domain;
    SystemEventState_t state;
    uint16_t code;
    uint16_t detail;
    uint16_t configuration_revision;
    uint32_t correlation_event_id;
} SystemEventRecord_t;

void SystemEventService_Initialize(void);

bool SystemEventService_Record(const SystemEventCapture_t *capture,
                               uint32_t *sequence);

uint16_t SystemEventService_GetCount(void);

/* age 0 is the newest retained event. */
bool SystemEventService_GetNewest(uint16_t age,
                                  SystemEventRecord_t *record);

/* index 0 is the oldest retained event. */
bool SystemEventService_GetOldest(uint16_t index,
                                  SystemEventRecord_t *record);

bool SystemEventService_GetBySequence(uint32_t sequence,
                                      SystemEventRecord_t *record);

/* Clears retained records without reusing a previously issued sequence. */
void SystemEventService_Clear(void);

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_EVENT_SERVICE_H */
