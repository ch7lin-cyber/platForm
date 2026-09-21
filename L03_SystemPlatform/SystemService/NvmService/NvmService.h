#ifndef NVM_SERVICE_H
#define NVM_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

#include "EventService.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    NVM_SERVICE_STATE_UNINITIALIZED = 0,
    NVM_SERVICE_STATE_IDLE,
    NVM_SERVICE_STATE_ERASE,
    NVM_SERVICE_STATE_WRITE_DATA,
    NVM_SERVICE_STATE_WRITE_COMMIT,
    NVM_SERVICE_STATE_VERIFY,
    NVM_SERVICE_STATE_COMPLETE,
    NVM_SERVICE_STATE_ERROR
} NvmServiceState_t;

bool NvmService_Initialize(void);
bool NvmService_QueueTemperatureInputConfiguration(
    uint16_t configuration_revision,
    const EventTemperatureInputConfiguration_t *configuration);
void NvmService_Process(void);
NvmServiceState_t NvmService_GetState(void);
bool NvmService_GetLoadedTemperatureInputConfiguration(
    uint16_t *configuration_revision,
    EventTemperatureInputConfiguration_t *configuration);
uint16_t NvmService_GetCompletedRevision(void);
void NvmService_AcknowledgeCompletion(void);

#ifdef __cplusplus
}
#endif
#endif
