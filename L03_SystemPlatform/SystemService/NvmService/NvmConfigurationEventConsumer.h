#ifndef NVM_CONFIGURATION_EVENT_CONSUMER_H
#define NVM_CONFIGURATION_EVENT_CONSUMER_H

#include <stdbool.h>
#include <stdint.h>

/* NVM fault Snapshot diagnostic value indexes. */
#define NVM_FAULT_SNAPSHOT_VALUE_STATE              (0U)
#define NVM_FAULT_SNAPSHOT_VALUE_ERROR              (1U)
#define NVM_FAULT_SNAPSHOT_VALUE_ACTIVE_SLOT        (2U)
#define NVM_FAULT_SNAPSHOT_VALUE_TARGET_SLOT        (3U)
#define NVM_FAULT_SNAPSHOT_VALUE_QUEUED_CHANNEL     (4U)
#define NVM_FAULT_SNAPSHOT_VALUE_SEQUENCE           (5U)
#define NVM_FAULT_SNAPSHOT_VALUE_LOADED_VALID_MASK  (6U)
#define NVM_FAULT_SNAPSHOT_VALUE_PENDING_VALID_MASK (7U)

bool NvmConfigurationEventConsumer_Initialize(void);
bool NvmConfigurationEventConsumer_Process(uint8_t channel,
                                           uint32_t timestamp_ms);

#endif
