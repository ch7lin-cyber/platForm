#ifndef FAULT_SERVICE_H
#define FAULT_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    FAULT_CODE_NONE = 0,
    FAULT_CODE_NVM_INITIALIZATION_FAILED = 0x0101,
    FAULT_CODE_NVM_ERASE_FAILED = 0x0102,
    FAULT_CODE_NVM_DATA_PROGRAM_FAILED = 0x0103,
    FAULT_CODE_NVM_COMMIT_PROGRAM_FAILED = 0x0104,
    FAULT_CODE_NVM_VERIFY_FAILED = 0x0105,
    FAULT_CODE_NVM_EVENT_ACK_FAILED = 0x0106
} FaultCode_t;

typedef struct
{
    FaultCode_t code;
    uint16_t first_detail;
    uint16_t last_detail;
    uint16_t first_configuration_revision;
    uint16_t last_configuration_revision;
    uint32_t first_event_id;
    uint32_t last_event_id;
    uint32_t occurrence_count;
    bool active;
} FaultRecord_t;

void FaultService_Initialize(void);
bool FaultService_Raise(FaultCode_t code, uint16_t detail,
                        uint16_t configuration_revision,
                        uint32_t event_id);
bool FaultService_Clear(FaultCode_t code);
bool FaultService_IsActive(FaultCode_t code);
bool FaultService_Get(FaultCode_t code, FaultRecord_t *record);
uint16_t FaultService_GetActiveCount(void);
bool FaultService_GetActiveByIndex(uint16_t active_index,
                                   FaultRecord_t *record);

#ifdef __cplusplus
}
#endif
#endif
