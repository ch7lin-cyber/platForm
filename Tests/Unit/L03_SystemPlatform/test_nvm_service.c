#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "EventService.h"
#include "HalNvm.h"
#include "NvmConfigurationEventConsumer.h"
#include "NvmService.h"

#define MOCK_SLOT_SIZE (32768U)
static uint8_t g_storage[HAL_NVM_SLOT_COUNT][MOCK_SLOT_SIZE];

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
        {MockInitialize, MockRead, MockErase, MockProgram};
    EventTemperatureInputConfiguration_t first = {0.5F, 95U, 48U};
    EventTemperatureInputConfiguration_t second = {2.0F, 95U, 46U};
    EventTemperatureInputConfiguration_t loaded;
    uint16_t revision;

    (void)memset(g_storage, 0xFF, sizeof(g_storage));
    assert(HalNvm_RegisterDriver(&ops, NULL) == HAL_NVM_STATUS_OK);
    assert(NvmService_Initialize());
    assert(!NvmService_GetLoadedTemperatureInputConfiguration(
        &revision, &loaded));
    assert(NvmService_QueueTemperatureInputConfiguration(1U, &first));
    ProcessToCompletion();
    NvmService_AcknowledgeCompletion();
    assert(NvmService_GetLoadedTemperatureInputConfiguration(
        &revision, &loaded));
    assert(revision == 1U);
    assert(loaded.sensor_type == 95U);
    assert(loaded.tc_linearization == 48U);

    assert(NvmService_QueueTemperatureInputConfiguration(2U, &second));
    NvmService_Process(); /* erase inactive slot */
    NvmService_Process(); /* write data, but no commit: simulated power loss */
    assert(NvmService_Initialize());
    assert(NvmService_GetLoadedTemperatureInputConfiguration(
        &revision, &loaded));
    assert(revision == 1U);
    assert(loaded.tc_linearization == 48U);

    assert(NvmService_QueueTemperatureInputConfiguration(2U, &second));
    ProcessToCompletion();
    assert(NvmService_GetLoadedTemperatureInputConfiguration(
        &revision, &loaded));
    assert(revision == 2U);
    assert(loaded.filter_time_constant_seconds == 2.0F);
    assert(loaded.tc_linearization == 46U);
    NvmService_AcknowledgeCompletion();
}

static void TestEventAcknowledgedAfterVerifiedWrite(void)
{
    EventTemperatureInputConfiguration_t old_configuration =
        {2.0F, 95U, 46U};
    EventTemperatureInputConfiguration_t new_configuration =
        {2.0F, 62U, 46U};
    uint8_t step;

    assert(EventService_Initialize(EVENT_ACK_SERIAL_REQUIRED_DEFAULT));
    assert(EventService_ConfigureTemperatureInputRequiredAckMask(EVENT_ACK_NVM));
    assert(NvmConfigurationEventConsumer_Initialize());
    assert(EventService_RaiseTemperatureInputConfigurationChanged(
        0U, 3U, EVENT_TEMPERATURE_INPUT_CHANGE_SENSOR_TYPE,
        &old_configuration, &new_configuration, NULL));
    for (step = 0U; step < 3U; step++)
    {
        assert(NvmConfigurationEventConsumer_Process(0U));
        assert(EventService_IsTemperatureInputConfigurationChangedPending(0U));
    }
    assert(NvmConfigurationEventConsumer_Process(0U));
    assert(!EventService_IsTemperatureInputConfigurationChangedPending(0U));
}

int main(void)
{
    TestDualSlotAndPowerLoss();
    TestEventAcknowledgedAfterVerifiedWrite();
    return 0;
}
