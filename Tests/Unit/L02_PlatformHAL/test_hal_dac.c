#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "HalDac.h"

typedef struct
{
    uint16_t written_code;
    unsigned int initialize_count;
    unsigned int write_count;
    HalDacStatus_t next_status;
} MockDac_t;

static HalDacStatus_t MockInitialize(void *context)
{
    MockDac_t *mock = (MockDac_t *)context;

    mock->initialize_count++;
    return mock->next_status;
}

static HalDacStatus_t MockWrite(void *context, uint16_t code)
{
    MockDac_t *mock = (MockDac_t *)context;

    mock->written_code = code;
    mock->write_count++;
    return mock->next_status;
}

int main(void)
{
    static const HalDacDriverOps_t ops = {MockInitialize, MockWrite};
    MockDac_t mock = {0U, 0U, 0U, HAL_DAC_STATUS_OK};
    uint16_t code = 0U;

    assert(HalDac_WriteCode(0U, 1U) == HAL_DAC_STATUS_NOT_REGISTERED);
    assert(HalDac_RegisterDriver(0U, &ops, &mock) == HAL_DAC_STATUS_OK);
    assert(HalDac_WriteCode(0U, 1U) == HAL_DAC_STATUS_NOT_INITIALIZED);
    assert(HalDac_Initialize(0U) == HAL_DAC_STATUS_OK);
    assert(mock.initialize_count == 1U);
    assert(HalDac_IsInitialized(0U));

    assert(HalDac_WriteCode(0U, 0xA55AU) == HAL_DAC_STATUS_OK);
    assert(mock.write_count == 1U);
    assert(mock.written_code == 0xA55AU);
    assert(HalDac_GetLastCode(0U, &code) == HAL_DAC_STATUS_OK);
    assert(code == 0xA55AU);

    mock.next_status = HAL_DAC_STATUS_IO_ERROR;
    assert(HalDac_WriteCode(0U, 0x1234U) == HAL_DAC_STATUS_IO_ERROR);
    assert(HalDac_GetLastCode(0U, &code) == HAL_DAC_STATUS_OK);
    assert(code == 0xA55AU);

    assert(HalDac_RegisterDriver(HAL_DAC_CHANNEL_COUNT, &ops, &mock) ==
           HAL_DAC_STATUS_INVALID_ARGUMENT);
    assert(HalDac_GetLastCode(0U, NULL) ==
           HAL_DAC_STATUS_INVALID_ARGUMENT);
    return 0;
}
