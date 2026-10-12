#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "EventService.h"
#include "FaultService.h"
#include "HalNvm.h"
#include "NvmConfigurationEventConsumer.h"
#include "NvmService.h"

#define MOCK_SLOT_SIZE (32768U)
static uint8_t g_storage[HAL_NVM_SLOT_COUNT][MOCK_SLOT_SIZE];
static bool g_fail_erase;

static HalNvmStatus_t MockInitialize(void *context)
{
    (void)context;
    return HAL_NVM_STATUS_OK;
}

static HalNvmStatus_t MockRead(void *context, uint8_t slot, uint32_t offset,
                               uint8_t *data, uint32_t length)
{
    (void)context;
    if ((offset + length) > MOCK_SLOT_SIZE)
    {
        return HAL_NVM_STATUS_IO_ERROR;
    }
    (void)memcpy(data, &g_storage[slot][offset], length);
    return HAL_NVM_STATUS_OK;
}

static HalNvmStatus_t MockErase(void *context, uint8_t slot)
{
    (void)context;
    if (g_fail_erase)
    {
        return HAL_NVM_STATUS_IO_ERROR;
    }
    (void)memset(g_storage[slot], 0xFF, MOCK_SLOT_SIZE);
    return HAL_NVM_STATUS_OK;
}

static HalNvmStatus_t MockProgram(void *context, uint8_t slot, uint8_t page,
                                  const uint8_t *data)
{
    uint32_t index;
    uint32_t offset = (uint32_t)page * HAL_NVM_PAGE_SIZE;
    (void)context;
    for (index = 0U; index < HAL_NVM_PAGE_SIZE; index++)
    {
        if ((g_storage[slot][offset + index] & data[index]) != data[index])
        {
            return HAL_NVM_STATUS_IO_ERROR;
        }
        g_storage[slot][offset + index] &= data[index];
    }
    return HAL_NVM_STATUS_OK;
}

static void ProcessToCompletion(void)
{
    uint8_t step;
    for (step = 0U; step < 4U; step++)
    {
        NvmService_Process();
    }
    assert(NvmService_GetState() == NVM_SERVICE_STATE_COMPLETE);
}

static void TestDualSlotAndPowerLoss(void)
{
    static const HalNvmDriverOps_t ops =
        {MockInitialize, MockRead, MockErase, MockProgram,
         NULL, NULL, NULL};
    EventTemperatureInputConfiguration_t first = {0.5F, 48U, EVENT_CJC_SOURCE_INTERNAL_TMP20};
    EventTemperatureInputConfiguration_t second = {2.0F, 46U, EVENT_CJC_SOURCE_INTERNAL_TMP20};
    EventTemperatureInputConfiguration_t channel_three = {1.0F, 124U, EVENT_CJC_SOURCE_INTERNAL_TMP20};
    EventTemperatureInputConfiguration_t loaded;
    uint16_t revision;

    (void)memset(g_storage, 0xFF, sizeof(g_storage));
    assert(HalNvm_RegisterDriver(&ops, NULL) == HAL_NVM_STATUS_OK);
    assert(NvmService_Initialize());
    {
        uint32_t capacity = 0U;
        uint8_t data = 0U;
        assert(HalNvm_GetCapacity(&capacity) ==
               HAL_NVM_STATUS_NOT_SUPPORTED);
        assert(HalNvm_ReadRaw(0U, &data, 1U) ==
               HAL_NVM_STATUS_NOT_SUPPORTED);
        assert(HalNvm_WriteRaw(0U, &data, 1U) ==
               HAL_NVM_STATUS_NOT_SUPPORTED);
    }
    assert(!NvmService_GetLoadedTemperatureInputConfiguration(
        &revision, &loaded));
    assert(NvmService_QueueTemperatureInputConfiguration(1U, &first));
    ProcessToCompletion();
    NvmService_AcknowledgeCompletion();
    assert(NvmService_GetLoadedTemperatureInputConfiguration(
        &revision, &loaded));
    assert(revision == 1U);
    assert(loaded.sensor_type == 48U);

    assert(NvmService_QueueTemperatureInputConfiguration(2U, &second));
    NvmService_Process(); /* erase inactive slot */
    NvmService_Process(); /* write data, but no commit: simulated power loss */
    assert(NvmService_Initialize());
    assert(NvmService_GetLoadedTemperatureInputConfiguration(
        &revision, &loaded));
    assert(revision == 1U);
    assert(loaded.sensor_type == 48U);

    assert(NvmService_QueueTemperatureInputConfiguration(2U, &second));
    ProcessToCompletion();
    assert(NvmService_GetLoadedTemperatureInputConfiguration(
        &revision, &loaded));
    assert(revision == 2U);
    assert(loaded.filter_time_constant_seconds == 2.0F);
    assert(loaded.sensor_type == 46U);
    NvmService_AcknowledgeCompletion();

    assert(NvmService_QueueTemperatureInputConfigurationForChannel(
        3U, 7U, &channel_three));
    ProcessToCompletion();
    assert(NvmService_GetCompletedChannel() == 3U);
    NvmService_AcknowledgeCompletion();
    assert(NvmService_Initialize());
    assert(NvmService_GetLoadedTemperatureInputConfigurationForChannel(
        3U, &revision, &loaded));
    assert(revision == 7U);
    assert(loaded.sensor_type == 124U);
    assert(NvmService_GetLoadedTemperatureInputConfigurationForChannel(
        0U, &revision, &loaded));
    assert(revision == 2U);
    assert(loaded.sensor_type == 46U);
}

static void TestEventAcknowledgedAfterVerifiedWrite(void)
{
    EventTemperatureInputConfiguration_t old_configuration =
        {2.0F, 46U, EVENT_CJC_SOURCE_INTERNAL_TMP20};
    EventTemperatureInputConfiguration_t new_configuration =
        {2.0F, 62U, EVENT_CJC_SOURCE_INTERNAL_TMP20};
    uint8_t step;

    assert(EventService_Initialize(EVENT_ACK_SERIAL_REQUIRED_DEFAULT));
    FaultService_Initialize();
    assert(EventService_ConfigureTemperatureInputRequiredAckMask(EVENT_ACK_NVM));
    assert(NvmConfigurationEventConsumer_Initialize());
    assert(EventService_RaiseTemperatureInputConfigurationChanged(
        0U, 3U, EVENT_TEMPERATURE_INPUT_CHANGE_SENSOR_TYPE,
        &old_configuration, &new_configuration, NULL));
    for (step = 0U; step < 3U; step++)
    {
        assert(NvmConfigurationEventConsumer_Process(0U, 0U));
        assert(EventService_IsTemperatureInputConfigurationChangedPending(0U));
    }
    assert(NvmConfigurationEventConsumer_Process(0U, 0U));
    assert(!EventService_IsTemperatureInputConfigurationChangedPending(0U));
}

static void TestEraseFailureRaisesFaultAndExplicitClearRetries(void)
{
    EventTemperatureInputConfiguration_t old_configuration =
        {2.0F, 62U, EVENT_CJC_SOURCE_INTERNAL_TMP20};
    EventTemperatureInputConfiguration_t new_configuration =
        {2.0F, 48U, EVENT_CJC_SOURCE_INTERNAL_TMP20};
    TemperatureInputConfigurationChangedEvent_t event;
    FaultRecord_t fault;
    uint8_t step;

    assert(EventService_Initialize(EVENT_ACK_SERIAL_REQUIRED_DEFAULT));
    assert(EventService_ConfigureTemperatureInputRequiredAckMask(EVENT_ACK_NVM));
    FaultService_Initialize();
    assert(NvmConfigurationEventConsumer_Initialize());
    assert(EventService_RaiseTemperatureInputConfigurationChanged(
        0U, 4U, EVENT_TEMPERATURE_INPUT_CHANGE_SENSOR_TYPE,
        &old_configuration, &new_configuration, NULL));
    assert(EventService_GetTemperatureInputConfigurationChanged(0U, &event));

    g_fail_erase = true;
    assert(!NvmConfigurationEventConsumer_Process(0U, 0U));
    assert(FaultService_IsActive(FAULT_CODE_NVM_ERASE_FAILED));
    assert(FaultService_Get(FAULT_CODE_NVM_ERASE_FAILED, &fault));
    assert(fault.first_configuration_revision == 4U);
    assert(fault.first_event_id == event.event_id);
    assert(fault.occurrence_count == 1U);
    assert(!NvmConfigurationEventConsumer_Process(0U, 0U));
    assert(FaultService_Get(FAULT_CODE_NVM_ERASE_FAILED, &fault));
    assert(fault.occurrence_count == 1U);
    assert(EventService_IsTemperatureInputConfigurationChangedPending(0U));

    g_fail_erase = false;
    assert(FaultService_Clear(FAULT_CODE_NVM_ERASE_FAILED));
    assert(!NvmConfigurationEventConsumer_Process(0U, 0U));
    for (step = 0U; step < 4U; step++)
    {
        assert(NvmConfigurationEventConsumer_Process(0U, 0U));
    }
    assert(!EventService_IsTemperatureInputConfigurationChangedPending(0U));
    assert(!FaultService_IsActive(FAULT_CODE_NVM_ERASE_FAILED));
}


static void TestPut16(uint8_t *p, uint16_t v) { p[0]=(uint8_t)v; p[1]=(uint8_t)(v>>8U); }
static void TestPut32(uint8_t *p, uint32_t v) { uint8_t i; for(i=0;i<4U;i++) p[i]=(uint8_t)(v>>(8U*i)); }
static uint32_t TestCrc(const uint8_t *p, uint32_t n)
{
    uint32_t c=0xFFFFFFFFUL,i; uint8_t b;
    for(i=0;i<n;i++) { c^=p[i]; for(b=0;b<8;b++) c=(c>>1)^((c&1U)?0xEDB88320UL:0U); }
    return ~c;
}
static void TestV3MigrationAndCjcPersistence(void)
{
    EventTemperatureInputConfiguration_t loaded;
    uint16_t revision;
    uint32_t bits,crc_offset=16U+8U*NVM_SERVICE_TEMPERATURE_INPUT_COUNT;
    float filter=0.5F;
    uint8_t *data=g_storage[0],*commit=&g_storage[0][HAL_NVM_PAGE_SIZE];
    (void)memset(g_storage,0xFF,sizeof(g_storage));
    TestPut32(data,0x54494E31UL); TestPut16(data+4,3U);
    TestPut16(data+6,(uint16_t)(2U+8U*NVM_SERVICE_TEMPERATURE_INPUT_COUNT));
    TestPut32(data+8,1U); TestPut16(data+12,NVM_SERVICE_TEMPERATURE_INPUT_COUNT); TestPut16(data+14,1U);
    TestPut16(data+16,1U); (void)memcpy(&bits,&filter,sizeof(bits));
    TestPut32(data+18,bits); TestPut16(data+22,48U);
    TestPut32(data+crc_offset,TestCrc(data,crc_offset));
    TestPut32(commit,0xA5A55A5AUL); TestPut32(commit+4,1U); TestPut32(commit+8,TestCrc(commit,8U));
    assert(NvmService_Initialize());
    assert(NvmService_GetLoadedTemperatureInputConfiguration(&revision,&loaded));
    assert(revision==1U && loaded.sensor_type==48U && loaded.filter_time_constant_seconds==0.5F);
    assert(loaded.cjc_source==EVENT_CJC_SOURCE_INTERNAL_TMP20);
    loaded.cjc_source=EVENT_CJC_SOURCE_DISABLED;
    assert(NvmService_QueueTemperatureInputConfiguration(2U,&loaded)); ProcessToCompletion();
    NvmService_AcknowledgeCompletion(); assert(NvmService_Initialize());
    assert(NvmService_GetLoadedTemperatureInputConfiguration(&revision,&loaded));
    assert(revision==2U && loaded.cjc_source==EVENT_CJC_SOURCE_DISABLED);
    loaded.cjc_source=EVENT_CJC_SOURCE_EXTERNAL_2;
    assert(NvmService_QueueTemperatureInputConfiguration(3U,&loaded)); ProcessToCompletion();
    NvmService_AcknowledgeCompletion(); assert(NvmService_Initialize());
    assert(NvmService_GetLoadedTemperatureInputConfiguration(&revision,&loaded));
    assert(revision==3U && loaded.cjc_source==EVENT_CJC_SOURCE_EXTERNAL_2);
}

int main(void)
{
    TestDualSlotAndPowerLoss();
    TestEventAcknowledgedAfterVerifiedWrite();
    TestEraseFailureRaisesFaultAndExplicitClearRetries();
    TestV3MigrationAndCjcPersistence();
    return 0;
}

