#ifndef WARNING_SERVICE_H
#define WARNING_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

#include "SnapshotService.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WARNING_SOURCE_ADC_COMMUNICATION   (1UL << 0)
#define WARNING_SOURCE_ADC_INTEGRITY       (1UL << 1)
#define WARNING_SOURCE_ADC_REFERENCE       (1UL << 2)
#define WARNING_SOURCE_ADC_CONVERSION      (1UL << 3)
#define WARNING_SOURCE_ADC_INPUT_VOLTAGE   (1UL << 4)
#define WARNING_SOURCE_ADC_STALE           (1UL << 5)
#define WARNING_SOURCE_LOW_VOLTAGE         (1UL << 6)
#define WARNING_SOURCE_MCU_TEMPERATURE     (1UL << 7)
#define WARNING_SOURCE_NVM                 (1UL << 8)
#define WARNING_SOURCE_ADC_INTERNAL        (1UL << 9)
#define WARNING_SOURCE_APPLICATION         (1UL << 15)
#define WARNING_SOURCE_ALL                 \
    (WARNING_SOURCE_ADC_COMMUNICATION |    \
     WARNING_SOURCE_ADC_INTEGRITY |        \
     WARNING_SOURCE_ADC_REFERENCE |        \
     WARNING_SOURCE_ADC_CONVERSION |       \
     WARNING_SOURCE_ADC_INPUT_VOLTAGE |    \
     WARNING_SOURCE_ADC_STALE |            \
     WARNING_SOURCE_LOW_VOLTAGE |          \
     WARNING_SOURCE_MCU_TEMPERATURE |      \
     WARNING_SOURCE_NVM |                  \
     WARNING_SOURCE_ADC_INTERNAL |         \
     WARNING_SOURCE_APPLICATION)

typedef struct
{
    uint16_t assert_count;
    uint16_t clear_count;
    bool latching;
} WarningSourceConfiguration_t;

typedef struct
{
    uint32_t active_source_mask;
    uint32_t latched_source_mask;
    uint32_t warning_source_mask;
    uint32_t transition_count;
} WarningServiceStatus_t;

void WarningService_Initialize(void);

/* source_mask must select exactly one supported warning source. */
bool WarningService_ConfigureSource(
    uint32_t source_mask,
    const WarningSourceConfiguration_t *configuration);

/*
 * Call once per monitoring period for the selected source. The warning is
 * asserted or cleared only after the configured number of consecutive
 * samples. A newly asserted warning is recorded by SnapshotService.
 */
bool WarningService_UpdateSource(
    uint32_t source_mask,
    bool condition_present,
    uint16_t detail,
    uint32_t timestamp_ms,
    uint16_t configuration_revision,
    uint32_t event_id,
    const int32_t values[SNAPSHOT_SERVICE_VALUE_COUNT]);

/* Clears inactive latched warnings. Active warnings cannot be reset. */
bool WarningService_Reset(uint32_t source_mask);

bool WarningService_GetStatus(WarningServiceStatus_t *status);
bool WarningService_IsActive(uint32_t source_mask);

#ifdef __cplusplus
}
#endif

#endif /* WARNING_SERVICE_H */
