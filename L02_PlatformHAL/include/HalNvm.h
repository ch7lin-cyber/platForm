#ifndef HAL_NVM_H
#define HAL_NVM_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HAL_NVM_SLOT_COUNT    (2U)
#define HAL_NVM_PAGE_SIZE     (512U)

typedef enum
{
    HAL_NVM_STATUS_OK = 0,
    HAL_NVM_STATUS_INVALID_ARGUMENT,
    HAL_NVM_STATUS_NOT_REGISTERED,
    HAL_NVM_STATUS_NOT_INITIALIZED,
    HAL_NVM_STATUS_IO_ERROR
} HalNvmStatus_t;

typedef struct
{
    HalNvmStatus_t (*initialize)(void *context);
    HalNvmStatus_t (*read)(void *context, uint8_t slot, uint32_t offset,
                           uint8_t *data, uint32_t length);
    HalNvmStatus_t (*erase_slot)(void *context, uint8_t slot);
    HalNvmStatus_t (*program_page)(void *context, uint8_t slot,
                                   uint8_t page, const uint8_t *data);
} HalNvmDriverOps_t;

HalNvmStatus_t HalNvm_RegisterDriver(const HalNvmDriverOps_t *ops,
                                     void *context);
HalNvmStatus_t HalNvm_Initialize(void);
HalNvmStatus_t HalNvm_Read(uint8_t slot, uint32_t offset,
                           uint8_t *data, uint32_t length);
HalNvmStatus_t HalNvm_EraseSlot(uint8_t slot);
HalNvmStatus_t HalNvm_ProgramPage(uint8_t slot, uint8_t page,
                                  const uint8_t *data);
bool HalNvm_IsInitialized(void);

#ifdef __cplusplus
}
#endif
#endif
