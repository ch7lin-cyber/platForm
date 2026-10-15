#ifndef NVM_SERVICE_H
#define NVM_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

#include "EventService.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NVM_SERVICE_TEMPERATURE_INPUT_COUNT (4U)
#define NVM_SERVICE_INVALID_SLOT             (0xFFU)
#define NVM_SERVICE_INVALID_CHANNEL          (0xFFU)

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

typedef enum
{
    NVM_SERVICE_ERROR_NONE = 0,
    NVM_SERVICE_ERROR_INITIALIZE,
    NVM_SERVICE_ERROR_ERASE,
    NVM_SERVICE_ERROR_WRITE_DATA,
    NVM_SERVICE_ERROR_WRITE_COMMIT,
    NVM_SERVICE_ERROR_VERIFY
} NvmServiceError_t;

typedef struct
{
    NvmServiceState_t state;
    NvmServiceError_t last_error;
    uint8_t active_slot;
    uint8_t target_slot;
    uint8_t queued_channel;
    uint32_t sequence;
    uint16_t loaded_valid_mask;
    uint16_t pending_valid_mask;
} NvmServiceDiagnostics_t;

bool NvmService_Initialize(void);
bool NvmService_QueueTemperatureInputConfiguration(
    uint16_t configuration_revision,
    const EventTemperatureInputConfiguration_t *configuration);
bool NvmService_QueueTemperatureInputConfigurationForChannel(
    uint8_t channel,
    uint16_t configuration_revision,
    const EventTemperatureInputConfiguration_t *configuration);
void NvmService_Process(void);
NvmServiceState_t NvmService_GetState(void);
NvmServiceError_t NvmService_GetLastError(void);
bool NvmService_GetDiagnostics(NvmServiceDiagnostics_t *diagnostics);
void NvmService_ResetError(void);
bool NvmService_GetLoadedTemperatureInputConfiguration(
    uint16_t *configuration_revision,
    EventTemperatureInputConfiguration_t *configuration);
bool NvmService_GetLoadedTemperatureInputConfigurationForChannel(
    uint8_t channel,
    uint16_t *configuration_revision,
    EventTemperatureInputConfiguration_t *configuration);
uint16_t NvmService_GetCompletedRevision(void);
uint8_t NvmService_GetCompletedChannel(void);
void NvmService_AcknowledgeCompletion(void);

#ifdef __cplusplus
}
#endif
#endif
