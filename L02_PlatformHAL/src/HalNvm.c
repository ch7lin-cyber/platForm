#include "HalNvm.h"

#include <stddef.h>

static const HalNvmDriverOps_t *g_ops;
static void *g_context;
static bool g_initialized;

HalNvmStatus_t HalNvm_RegisterDriver(const HalNvmDriverOps_t *ops,
                                     void *context)
{
    if ((ops == NULL) || (ops->initialize == NULL) ||
        (ops->read == NULL) || (ops->erase_slot == NULL) ||
        (ops->program_page == NULL))
    {
        return HAL_NVM_STATUS_INVALID_ARGUMENT;
    }
    g_ops = ops;
    g_context = context;
    g_initialized = false;
    return HAL_NVM_STATUS_OK;
}

HalNvmStatus_t HalNvm_Initialize(void)
{
    HalNvmStatus_t status;
    if (g_ops == NULL)
    {
        return HAL_NVM_STATUS_NOT_REGISTERED;
    }
    status = g_ops->initialize(g_context);
    g_initialized = (status == HAL_NVM_STATUS_OK);
    return status;
}

HalNvmStatus_t HalNvm_Read(uint8_t slot, uint32_t offset,
                           uint8_t *data, uint32_t length)
{
    if ((slot >= HAL_NVM_SLOT_COUNT) || (data == NULL) || (length == 0U))
    {
        return HAL_NVM_STATUS_INVALID_ARGUMENT;
    }
    if (!g_initialized)
    {
        return HAL_NVM_STATUS_NOT_INITIALIZED;
    }
    return g_ops->read(g_context, slot, offset, data, length);
}

HalNvmStatus_t HalNvm_EraseSlot(uint8_t slot)
{
    if (slot >= HAL_NVM_SLOT_COUNT)
    {
        return HAL_NVM_STATUS_INVALID_ARGUMENT;
    }
    if (!g_initialized)
    {
        return HAL_NVM_STATUS_NOT_INITIALIZED;
    }
    return g_ops->erase_slot(g_context, slot);
}

HalNvmStatus_t HalNvm_ProgramPage(uint8_t slot, uint8_t page,
                                  const uint8_t *data)
{
    if ((slot >= HAL_NVM_SLOT_COUNT) || (page > 1U) || (data == NULL))
    {
        return HAL_NVM_STATUS_INVALID_ARGUMENT;
    }
    if (!g_initialized)
    {
        return HAL_NVM_STATUS_NOT_INITIALIZED;
    }
    return g_ops->program_page(g_context, slot, page, data);
}

bool HalNvm_IsInitialized(void)
{
    return g_initialized;
}
