#include "NvmService.h"

#include <stddef.h>
#include <string.h>

#include "HalNvm.h"

#define NVM_DATA_MAGIC          (0x54494E31UL)
#define NVM_COMMIT_MAGIC        (0xA5A55A5AUL)
#define NVM_FORMAT_VERSION      (1U)
#define NVM_PAYLOAD_LENGTH      (8U)
#define NVM_DATA_CRC_OFFSET     (24U)
#define NVM_COMMIT_CRC_OFFSET   (8U)

static NvmServiceState_t g_state;
static EventTemperatureInputConfiguration_t g_configuration;
static EventTemperatureInputConfiguration_t g_loaded_configuration;
static uint16_t g_revision;
static uint16_t g_loaded_revision;
static uint16_t g_completed_revision;
static uint32_t g_sequence;
static uint8_t g_active_slot;
static uint8_t g_target_slot;
static bool g_loaded_valid;
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
    (void)memset(g_data_page, 0xFF, sizeof(g_data_page));
    (void)memset(g_commit_page, 0xFF, sizeof(g_commit_page));
    PutU32(&g_data_page[0], NVM_DATA_MAGIC);
    PutU16(&g_data_page[4], NVM_FORMAT_VERSION);
    PutU16(&g_data_page[6], NVM_PAYLOAD_LENGTH);
    PutU32(&g_data_page[8], g_sequence);
    PutU16(&g_data_page[12], g_revision);
    PutU16(&g_data_page[14], 0U);
    (void)memcpy(&filter_bits,
                 &g_configuration.filter_time_constant_seconds,
                 sizeof(filter_bits));
    PutU32(&g_data_page[16], filter_bits);
    PutU16(&g_data_page[20], g_configuration.sensor_type);
    PutU16(&g_data_page[22], g_configuration.tc_linearization);
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
    uint16_t *revision,
    EventTemperatureInputConfiguration_t *configuration)
{
    uint8_t data_page[HAL_NVM_PAGE_SIZE];
    uint8_t commit_page[HAL_NVM_PAGE_SIZE];
    uint32_t filter_bits;
    if ((HalNvm_Read(slot, 0U, data_page, sizeof(data_page)) !=
         HAL_NVM_STATUS_OK) ||
        (HalNvm_Read(slot, HAL_NVM_PAGE_SIZE, commit_page,
                     sizeof(commit_page)) != HAL_NVM_STATUS_OK))
    {
        return false;
    }
    if ((GetU32(&data_page[0]) != NVM_DATA_MAGIC) ||
        (GetU16(&data_page[4]) != NVM_FORMAT_VERSION) ||
        (GetU16(&data_page[6]) != NVM_PAYLOAD_LENGTH) ||
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
    *revision = GetU16(&data_page[12]);
    filter_bits = GetU32(&data_page[16]);
    (void)memcpy(&configuration->filter_time_constant_seconds,
                 &filter_bits, sizeof(filter_bits));
    configuration->sensor_type = GetU16(&data_page[20]);
    configuration->tc_linearization = GetU16(&data_page[22]);
    return true;
}

bool NvmService_Initialize(void)
{
    uint8_t slot;
    uint32_t sequence;
    uint16_t revision;
    EventTemperatureInputConfiguration_t configuration;

    if (HalNvm_Initialize() != HAL_NVM_STATUS_OK)
    {
        g_state = NVM_SERVICE_STATE_ERROR;
        return false;
    }
    g_loaded_valid = false;
    g_sequence = 0U;
    g_active_slot = 1U;
    for (slot = 0U; slot < HAL_NVM_SLOT_COUNT; slot++)
    {
        if (ReadValidSlot(slot, &sequence, &revision, &configuration) &&
            (!g_loaded_valid || ((int32_t)(sequence - g_sequence) > 0)))
        {
            g_loaded_valid = true;
            g_sequence = sequence;
            g_loaded_revision = revision;
            g_loaded_configuration = configuration;
            g_active_slot = slot;
        }
    }
    g_completed_revision = 0U;
    g_state = NVM_SERVICE_STATE_IDLE;
    return true;
}

bool NvmService_QueueTemperatureInputConfiguration(
    uint16_t configuration_revision,
    const EventTemperatureInputConfiguration_t *configuration)
{
    if ((g_state != NVM_SERVICE_STATE_IDLE) ||
        (configuration_revision == 0U) || (configuration == NULL))
    {
        return false;
    }
    g_revision = configuration_revision;
    g_configuration = *configuration;
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
    uint16_t revision;
    EventTemperatureInputConfiguration_t configuration;

    switch (g_state)
    {
        case NVM_SERVICE_STATE_ERASE:
            g_state = (HalNvm_EraseSlot(g_target_slot) == HAL_NVM_STATUS_OK) ?
                NVM_SERVICE_STATE_WRITE_DATA : NVM_SERVICE_STATE_ERROR;
            break;
        case NVM_SERVICE_STATE_WRITE_DATA:
            g_state = (HalNvm_ProgramPage(g_target_slot, 0U, g_data_page) ==
                       HAL_NVM_STATUS_OK) ? NVM_SERVICE_STATE_WRITE_COMMIT :
                       NVM_SERVICE_STATE_ERROR;
            break;
        case NVM_SERVICE_STATE_WRITE_COMMIT:
            g_state = (HalNvm_ProgramPage(g_target_slot, 1U, g_commit_page) ==
                       HAL_NVM_STATUS_OK) ? NVM_SERVICE_STATE_VERIFY :
                       NVM_SERVICE_STATE_ERROR;
            break;
        case NVM_SERVICE_STATE_VERIFY:
            if (ReadValidSlot(g_target_slot, &sequence, &revision,
                              &configuration) &&
                (sequence == g_sequence) && (revision == g_revision) &&
                (memcmp(&configuration, &g_configuration,
                        sizeof(configuration)) == 0))
            {
                g_active_slot = g_target_slot;
                g_loaded_valid = true;
                g_loaded_revision = revision;
                g_loaded_configuration = configuration;
                g_completed_revision = revision;
                g_state = NVM_SERVICE_STATE_COMPLETE;
            }
            else
            {
                g_state = NVM_SERVICE_STATE_ERROR;
            }
            break;
        default:
            break;
    }
}

NvmServiceState_t NvmService_GetState(void) { return g_state; }

bool NvmService_GetLoadedTemperatureInputConfiguration(
    uint16_t *configuration_revision,
    EventTemperatureInputConfiguration_t *configuration)
{
    if (!g_loaded_valid || (configuration_revision == NULL) ||
        (configuration == NULL))
    {
        return false;
    }
    *configuration_revision = g_loaded_revision;
    *configuration = g_loaded_configuration;
    return true;
}

uint16_t NvmService_GetCompletedRevision(void)
{
    return g_completed_revision;
}

void NvmService_AcknowledgeCompletion(void)
{
    if (g_state == NVM_SERVICE_STATE_COMPLETE)
    {
        g_completed_revision = 0U;
        g_state = NVM_SERVICE_STATE_IDLE;
    }
}
