#include "NvmService.h"

#include <stddef.h>
#include <string.h>

#include "HalNvm.h"

#define NVM_DATA_MAGIC          (0x54494E31UL)
#define NVM_COMMIT_MAGIC        (0xA5A55A5AUL)
#define NVM_FORMAT_VERSION      (2U)
#define NVM_ENTRY_LENGTH        (10U)
#define NVM_PAYLOAD_LENGTH      \
    (2U + (NVM_SERVICE_TEMPERATURE_INPUT_COUNT * NVM_ENTRY_LENGTH))
#define NVM_DATA_CRC_OFFSET     \
    (16U + (NVM_SERVICE_TEMPERATURE_INPUT_COUNT * NVM_ENTRY_LENGTH))
#define NVM_COMMIT_CRC_OFFSET   (8U)

static NvmServiceState_t g_state;
static NvmServiceError_t g_last_error;
static EventTemperatureInputConfiguration_t
    g_pending_configuration[NVM_SERVICE_TEMPERATURE_INPUT_COUNT];
static EventTemperatureInputConfiguration_t
    g_loaded_configuration[NVM_SERVICE_TEMPERATURE_INPUT_COUNT];
static uint16_t g_pending_revision[NVM_SERVICE_TEMPERATURE_INPUT_COUNT];
static uint16_t g_loaded_revision[NVM_SERVICE_TEMPERATURE_INPUT_COUNT];
static uint16_t g_pending_valid_mask;
static uint16_t g_loaded_valid_mask;
static uint8_t g_queued_channel;
static uint8_t g_completed_channel;
static uint16_t g_completed_revision;
static uint32_t g_sequence;
static uint8_t g_active_slot;
static uint8_t g_target_slot;
typedef union
{
    uint32_t alignment;
    uint8_t bytes[HAL_NVM_PAGE_SIZE];
} NvmPageBuffer_t;

static NvmPageBuffer_t g_data_page_buffer;
static NvmPageBuffer_t g_commit_page_buffer;
#define g_data_page   (g_data_page_buffer.bytes)
#define g_commit_page (g_commit_page_buffer.bytes)

static void PutU16(uint8_t *data, uint16_t value)
{
    data[0] = (uint8_t)value;
    data[1] = (uint8_t)(value >> 8U);
}

static uint16_t GetU16(const uint8_t *data)
{
    return (uint16_t)((uint16_t)data[0] | ((uint16_t)data[1] << 8U));
}

static void PutU32(uint8_t *data, uint32_t value)
{
    data[0] = (uint8_t)value;
    data[1] = (uint8_t)(value >> 8U);
    data[2] = (uint8_t)(value >> 16U);
    data[3] = (uint8_t)(value >> 24U);
}

static uint32_t GetU32(const uint8_t *data)
{
    return (uint32_t)data[0] | ((uint32_t)data[1] << 8U) |
           ((uint32_t)data[2] << 16U) | ((uint32_t)data[3] << 24U);
}

static uint32_t Crc32(const uint8_t *data, uint32_t length)
{
    uint32_t crc = 0xFFFFFFFFUL;
    uint32_t index;
    uint8_t bit;
    for (index = 0U; index < length; index++)
    {
        crc ^= data[index];
        for (bit = 0U; bit < 8U; bit++)
        {
            crc = (crc >> 1U) ^
                  ((crc & 1U) ? 0xEDB88320UL : 0U);
        }
    }
    return ~crc;
}

static void BuildPages(void)
{
    uint32_t filter_bits;
    uint8_t channel;
    (void)memset(g_data_page, 0xFF, sizeof(g_data_page));
    (void)memset(g_commit_page, 0xFF, sizeof(g_commit_page));
    PutU32(&g_data_page[0], NVM_DATA_MAGIC);
    PutU16(&g_data_page[4], NVM_FORMAT_VERSION);
    PutU16(&g_data_page[6], (uint16_t)NVM_PAYLOAD_LENGTH);
    PutU32(&g_data_page[8], g_sequence);
    PutU16(&g_data_page[12], NVM_SERVICE_TEMPERATURE_INPUT_COUNT);
    PutU16(&g_data_page[14], g_pending_valid_mask);
    for (channel = 0U; channel < NVM_SERVICE_TEMPERATURE_INPUT_COUNT;
         channel++)
    {
        uint32_t offset = 16U + ((uint32_t)channel * NVM_ENTRY_LENGTH);
        PutU16(&g_data_page[offset], g_pending_revision[channel]);
        (void)memcpy(&filter_bits,
                     &g_pending_configuration[channel]
                          .filter_time_constant_seconds,
                     sizeof(filter_bits));
        PutU32(&g_data_page[offset + 2U], filter_bits);
        PutU16(&g_data_page[offset + 6U],
               g_pending_configuration[channel].sensor_type);
        PutU16(&g_data_page[offset + 8U],
               g_pending_configuration[channel].tc_linearization);
    }
    PutU32(&g_data_page[NVM_DATA_CRC_OFFSET],
           Crc32(g_data_page, NVM_DATA_CRC_OFFSET));

    PutU32(&g_commit_page[0], NVM_COMMIT_MAGIC);
    PutU32(&g_commit_page[4], g_sequence);
    PutU32(&g_commit_page[NVM_COMMIT_CRC_OFFSET],
           Crc32(g_commit_page, NVM_COMMIT_CRC_OFFSET));
}

static bool ReadValidSlot(
    uint8_t slot,
    uint32_t *sequence,
    uint16_t *valid_mask,
    uint16_t *revisions,
    EventTemperatureInputConfiguration_t *configurations)
{
    uint8_t data_page[HAL_NVM_PAGE_SIZE];
    uint8_t commit_page[HAL_NVM_PAGE_SIZE];
    uint8_t channel;
    if ((HalNvm_Read(slot, 0U, data_page, sizeof(data_page)) !=
         HAL_NVM_STATUS_OK) ||
        (HalNvm_Read(slot, HAL_NVM_PAGE_SIZE, commit_page,
                     sizeof(commit_page)) != HAL_NVM_STATUS_OK))
    {
        return false;
    }
    if ((GetU32(&data_page[0]) != NVM_DATA_MAGIC) ||
        (GetU16(&data_page[4]) != NVM_FORMAT_VERSION) ||
        (GetU16(&data_page[6]) != (uint16_t)NVM_PAYLOAD_LENGTH) ||
        (GetU16(&data_page[12]) !=
         NVM_SERVICE_TEMPERATURE_INPUT_COUNT) ||
        (GetU32(&data_page[NVM_DATA_CRC_OFFSET]) !=
         Crc32(data_page, NVM_DATA_CRC_OFFSET)) ||
        (GetU32(&commit_page[0]) != NVM_COMMIT_MAGIC) ||
        (GetU32(&commit_page[NVM_COMMIT_CRC_OFFSET]) !=
         Crc32(commit_page, NVM_COMMIT_CRC_OFFSET)) ||
        (GetU32(&data_page[8]) != GetU32(&commit_page[4])))
    {
        return false;
    }
    *sequence = GetU32(&data_page[8]);
    *valid_mask = GetU16(&data_page[14]);
    for (channel = 0U; channel < NVM_SERVICE_TEMPERATURE_INPUT_COUNT;
         channel++)
    {
        uint32_t offset = 16U + ((uint32_t)channel * NVM_ENTRY_LENGTH);
        uint32_t filter_bits = GetU32(&data_page[offset + 2U]);
        revisions[channel] = GetU16(&data_page[offset]);
        (void)memcpy(&configurations[channel].filter_time_constant_seconds,
                     &filter_bits, sizeof(filter_bits));
        configurations[channel].sensor_type =
            GetU16(&data_page[offset + 6U]);
        configurations[channel].tc_linearization =
            GetU16(&data_page[offset + 8U]);
    }
    return true;
}

bool NvmService_Initialize(void)
{
    uint8_t slot;
    uint32_t sequence;
    uint16_t valid_mask;
    uint16_t revisions[NVM_SERVICE_TEMPERATURE_INPUT_COUNT];
    EventTemperatureInputConfiguration_t
        configurations[NVM_SERVICE_TEMPERATURE_INPUT_COUNT];
    bool loaded = false;

    if (HalNvm_Initialize() != HAL_NVM_STATUS_OK)
    {
        g_last_error = NVM_SERVICE_ERROR_INITIALIZE;
        g_state = NVM_SERVICE_STATE_ERROR;
        return false;
    }
    (void)memset(g_loaded_configuration, 0,
                 sizeof(g_loaded_configuration));
    (void)memset(g_loaded_revision, 0, sizeof(g_loaded_revision));
    g_loaded_valid_mask = 0U;
    g_sequence = 0U;
    g_active_slot = 1U;
    for (slot = 0U; slot < HAL_NVM_SLOT_COUNT; slot++)
    {
        if (ReadValidSlot(slot, &sequence, &valid_mask, revisions,
                          configurations) &&
            (!loaded || ((int32_t)(sequence - g_sequence) > 0)))
        {
            loaded = true;
            g_sequence = sequence;
            g_loaded_valid_mask = valid_mask;
            (void)memcpy(g_loaded_revision, revisions,
                         sizeof(g_loaded_revision));
            (void)memcpy(g_loaded_configuration, configurations,
                         sizeof(g_loaded_configuration));
            g_active_slot = slot;
        }
    }
    (void)memcpy(g_pending_revision, g_loaded_revision,
                 sizeof(g_pending_revision));
    (void)memcpy(g_pending_configuration, g_loaded_configuration,
                 sizeof(g_pending_configuration));
    g_pending_valid_mask = g_loaded_valid_mask;
    g_queued_channel = 0U;
    g_completed_channel = 0U;
    g_completed_revision = 0U;
    g_last_error = NVM_SERVICE_ERROR_NONE;
    g_state = NVM_SERVICE_STATE_IDLE;
    return true;
}

bool NvmService_QueueTemperatureInputConfiguration(
    uint16_t configuration_revision,
    const EventTemperatureInputConfiguration_t *configuration)
{
    return NvmService_QueueTemperatureInputConfigurationForChannel(
        0U, configuration_revision, configuration);
}

bool NvmService_QueueTemperatureInputConfigurationForChannel(
    uint8_t channel,
    uint16_t configuration_revision,
    const EventTemperatureInputConfiguration_t *configuration)
{
    if ((g_state != NVM_SERVICE_STATE_IDLE) ||
        (channel >= NVM_SERVICE_TEMPERATURE_INPUT_COUNT) ||
        (configuration_revision == 0U) || (configuration == NULL))
    {
        return false;
    }
    (void)memcpy(g_pending_revision, g_loaded_revision,
                 sizeof(g_pending_revision));
    (void)memcpy(g_pending_configuration, g_loaded_configuration,
                 sizeof(g_pending_configuration));
    g_pending_valid_mask = g_loaded_valid_mask;
    g_pending_revision[channel] = configuration_revision;
    g_pending_configuration[channel] = *configuration;
    g_pending_valid_mask |= (uint16_t)(1UL << channel);
    g_queued_channel = channel;
    g_sequence++;
    if (g_sequence == 0U)
    {
        g_sequence = 1U;
    }
    g_target_slot = (uint8_t)(g_active_slot ^ 1U);
    BuildPages();
    g_state = NVM_SERVICE_STATE_ERASE;
    return true;
}

void NvmService_Process(void)
{
    uint32_t sequence;
    uint16_t valid_mask;
    uint16_t revisions[NVM_SERVICE_TEMPERATURE_INPUT_COUNT];
    EventTemperatureInputConfiguration_t
        configurations[NVM_SERVICE_TEMPERATURE_INPUT_COUNT];

    switch (g_state)
    {
        case NVM_SERVICE_STATE_ERASE:
            if (HalNvm_EraseSlot(g_target_slot) == HAL_NVM_STATUS_OK)
            {
                g_state = NVM_SERVICE_STATE_WRITE_DATA;
            }
            else
            {
                g_last_error = NVM_SERVICE_ERROR_ERASE;
                g_state = NVM_SERVICE_STATE_ERROR;
            }
            break;
        case NVM_SERVICE_STATE_WRITE_DATA:
            if (HalNvm_ProgramPage(g_target_slot, 0U, g_data_page) ==
                HAL_NVM_STATUS_OK)
            {
                g_state = NVM_SERVICE_STATE_WRITE_COMMIT;
            }
            else
            {
                g_last_error = NVM_SERVICE_ERROR_WRITE_DATA;
                g_state = NVM_SERVICE_STATE_ERROR;
            }
            break;
        case NVM_SERVICE_STATE_WRITE_COMMIT:
            if (HalNvm_ProgramPage(g_target_slot, 1U, g_commit_page) ==
                HAL_NVM_STATUS_OK)
            {
                g_state = NVM_SERVICE_STATE_VERIFY;
            }
            else
            {
                g_last_error = NVM_SERVICE_ERROR_WRITE_COMMIT;
                g_state = NVM_SERVICE_STATE_ERROR;
            }
            break;
        case NVM_SERVICE_STATE_VERIFY:
            if (ReadValidSlot(g_target_slot, &sequence, &valid_mask,
                              revisions, configurations) &&
                (sequence == g_sequence) &&
                (valid_mask == g_pending_valid_mask) &&
                (memcmp(revisions, g_pending_revision,
                        sizeof(revisions)) == 0) &&
                (memcmp(configurations, g_pending_configuration,
                        sizeof(configurations)) == 0))
            {
                g_active_slot = g_target_slot;
                g_loaded_valid_mask = valid_mask;
                (void)memcpy(g_loaded_revision, revisions,
                             sizeof(g_loaded_revision));
                (void)memcpy(g_loaded_configuration, configurations,
                             sizeof(g_loaded_configuration));
                g_completed_channel = g_queued_channel;
                g_completed_revision =
                    revisions[g_completed_channel];
                g_state = NVM_SERVICE_STATE_COMPLETE;
            }
            else
            {
                g_last_error = NVM_SERVICE_ERROR_VERIFY;
                g_state = NVM_SERVICE_STATE_ERROR;
            }
            break;
        default:
            break;
    }
}

NvmServiceState_t NvmService_GetState(void) { return g_state; }

NvmServiceError_t NvmService_GetLastError(void) { return g_last_error; }

void NvmService_ResetError(void)
{
    if (g_state == NVM_SERVICE_STATE_ERROR)
    {
        g_last_error = NVM_SERVICE_ERROR_NONE;
        g_state = NVM_SERVICE_STATE_IDLE;
    }
}

bool NvmService_GetLoadedTemperatureInputConfiguration(
    uint16_t *configuration_revision,
    EventTemperatureInputConfiguration_t *configuration)
{
    return NvmService_GetLoadedTemperatureInputConfigurationForChannel(
        0U, configuration_revision, configuration);
}

bool NvmService_GetLoadedTemperatureInputConfigurationForChannel(
    uint8_t channel,
    uint16_t *configuration_revision,
    EventTemperatureInputConfiguration_t *configuration)
{
    if ((channel >= NVM_SERVICE_TEMPERATURE_INPUT_COUNT) ||
        ((g_loaded_valid_mask & (uint16_t)(1UL << channel)) == 0U) ||
        (configuration_revision == NULL) ||
        (configuration == NULL))
    {
        return false;
    }
    *configuration_revision = g_loaded_revision[channel];
    *configuration = g_loaded_configuration[channel];
    return true;
}

uint16_t NvmService_GetCompletedRevision(void)
{
    return g_completed_revision;
}

uint8_t NvmService_GetCompletedChannel(void)
{
    return g_completed_channel;
}

void NvmService_AcknowledgeCompletion(void)
{
    if (g_state == NVM_SERVICE_STATE_COMPLETE)
    {
        g_completed_revision = 0U;
        g_state = NVM_SERVICE_STATE_IDLE;
    }
}
