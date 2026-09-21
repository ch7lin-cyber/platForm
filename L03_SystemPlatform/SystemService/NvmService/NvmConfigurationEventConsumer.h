#ifndef NVM_CONFIGURATION_EVENT_CONSUMER_H
#define NVM_CONFIGURATION_EVENT_CONSUMER_H

#include <stdbool.h>
#include <stdint.h>

bool NvmConfigurationEventConsumer_Initialize(void);
bool NvmConfigurationEventConsumer_Process(uint8_t channel);

#endif
