#include "FaultService.h"

#include <stddef.h>
#include <string.h>

#define FAULT_SERVICE_RECORD_COUNT (6U)

static FaultRecord_t g_records[FAULT_SERVICE_RECORD_COUNT];

static int16_t FindRecord(FaultCode_t code)
{
    uint16_t index;
    for (index = 0U; index < FAULT_SERVICE_RECORD_COUNT; index++)
    {
        if (g_records[index].code == code)
        {
            return (int16_t)index;
        }
    }
    return -1;
}

void FaultService_Initialize(void)
{
    static const FaultCode_t codes[FAULT_SERVICE_RECORD_COUNT] =
    {
        FAULT_CODE_NVM_INITIALIZATION_FAILED,
        FAULT_CODE_NVM_ERASE_FAILED,
        FAULT_CODE_NVM_DATA_PROGRAM_FAILED,
        FAULT_CODE_NVM_COMMIT_PROGRAM_FAILED,
        FAULT_CODE_NVM_VERIFY_FAILED,
        FAULT_CODE_NVM_EVENT_ACK_FAILED
    };
    uint16_t index;
    (void)memset(g_records, 0, sizeof(g_records));
    for (index = 0U; index < FAULT_SERVICE_RECORD_COUNT; index++)
    {
        g_records[index].code = codes[index];
    }
}

bool FaultService_Raise(FaultCode_t code, uint16_t detail,
                        uint16_t configuration_revision,
                        uint32_t event_id)
{
    int16_t index = FindRecord(code);
    FaultRecord_t *record;
    if (index < 0)
    {
        return false;
    }
    record = &g_records[(uint16_t)index];
    if (!record->active)
    {
        record->first_detail = detail;
        record->first_configuration_revision = configuration_revision;
        record->first_event_id = event_id;
        record->occurrence_count = 0U;
    }
    record->last_detail = detail;
    record->last_configuration_revision = configuration_revision;
    record->last_event_id = event_id;
    record->occurrence_count++;
    record->active = true;
    return true;
}

bool FaultService_Clear(FaultCode_t code)
{
    int16_t index = FindRecord(code);
    FaultCode_t saved_code;
    if (index < 0)
    {
        return false;
    }
    saved_code = g_records[(uint16_t)index].code;
    (void)memset(&g_records[(uint16_t)index], 0,
                 sizeof(g_records[(uint16_t)index]));
    g_records[(uint16_t)index].code = saved_code;
    return true;
}

bool FaultService_IsActive(FaultCode_t code)
{
    int16_t index = FindRecord(code);
    return (index >= 0) && g_records[(uint16_t)index].active;
}

bool FaultService_Get(FaultCode_t code, FaultRecord_t *record)
{
    int16_t index = FindRecord(code);
    if ((index < 0) || (record == NULL))
    {
        return false;
    }
    *record = g_records[(uint16_t)index];
    return true;
}

uint16_t FaultService_GetActiveCount(void)
{
    uint16_t index;
    uint16_t count = 0U;
    for (index = 0U; index < FAULT_SERVICE_RECORD_COUNT; index++)
    {
        if (g_records[index].active)
        {
            count++;
        }
    }
    return count;
}

bool FaultService_GetActiveByIndex(uint16_t active_index,
                                   FaultRecord_t *record)
{
    uint16_t index;
    uint16_t current = 0U;
    if (record == NULL)
    {
        return false;
    }
    for (index = 0U; index < FAULT_SERVICE_RECORD_COUNT; index++)
    {
        if (g_records[index].active)
        {
            if (current == active_index)
            {
                *record = g_records[index];
                return true;
            }
            current++;
        }
    }
    return false;
}
