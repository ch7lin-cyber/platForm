#ifndef SAFETY_SERVICE_H
#define SAFETY_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

#include "SnapshotService.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SAFETY_SOURCE_MCU_OVERTEMPERATURE (1UL << 0)
#define SAFETY_SOURCE_LOW_VOLTAGE         (1UL << 1)
#define SAFETY_SOURCE_SENSOR_FAULT        (1UL << 2)
#define SAFETY_SOURCE_WATCHDOG            (1UL << 3)
#define SAFETY_SOURCE_EXTERNAL_INTERLOCK  (1UL << 4)
#define SAFETY_SOURCE_APPLICATION         (1UL << 15)
#define SAFETY_SOURCE_ALL                 \
    (SAFETY_SOURCE_MCU_OVERTEMPERATURE |  \
     SAFETY_SOURCE_LOW_VOLTAGE |          \
     SAFETY_SOURCE_SENSOR_FAULT |         \
     SAFETY_SOURCE_WATCHDOG |             \
     SAFETY_SOURCE_EXTERNAL_INTERLOCK |   \
     SAFETY_SOURCE_APPLICATION)

typedef enum
{
    SAFETY_STATE_NORMAL = 0,
    SAFETY_STATE_TRIPPED,
    SAFETY_STATE_ACTION_ERROR
} SafetyState_t;

typedef enum
{
    SAFETY_RESET_OK = 0,
    SAFETY_RESET_BLOCKED_ACTIVE,
    SAFETY_RESET_INVALID_ARGUMENT
} SafetyResetResult_t;

typedef struct
{
    uint32_t active_source_mask;
    uint32_t latched_source_mask;
    uint32_t trip_source_mask;
    uint32_t latching_source_mask;
    uint32_t transition_count;
    SafetyState_t state;
    bool output_inhibited;
} SafetyServiceStatus_t;

typedef bool (*SafetyOutputAction_t)(bool inhibit,
                                     uint32_t trip_source_mask,
                                     void *context);

bool SafetyService_Initialize(uint32_t latching_source_mask,
                              SafetyOutputAction_t output_action,
                              void *output_action_context);

/*
 * Updates one or more safety sources. A rising source is captured in the
 * SnapshotService immediately. Call SafetyService_Process() afterwards to
 * apply the aggregate safe-output state.
 */
bool SafetyService_UpdateSource(
    uint32_t source_mask,
    bool active,
    uint16_t detail,
    uint32_t timestamp_ms,
    uint16_t configuration_revision,
    uint32_t event_id,
    const int32_t values[SNAPSHOT_SERVICE_VALUE_COUNT]);

bool SafetyService_Process(void);

SafetyResetResult_t SafetyService_Reset(uint32_t source_mask);

bool SafetyService_GetStatus(SafetyServiceStatus_t *status);
bool SafetyService_IsOutputInhibited(void);

#ifdef __cplusplus
}
#endif

#endif /* SAFETY_SERVICE_H */
