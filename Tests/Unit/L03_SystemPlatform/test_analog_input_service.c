#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "AnalogInputService.h"
#include "HalAdc.h"

typedef struct
{
    HalAdcStatus_t initialize_status;
    HalAdcStatus_t read_status;
    uint32_t raw_code;
    uint8_t channel;
    uint32_t initialize_count;
} MockAdcDriver_t;

static HalAdcStatus_t MockInitialize(void *context)
{
    MockAdcDriver_t *driver = (MockAdcDriver_t *)context;
    driver->initialize_count++;
    return driver->initialize_status;
}

static HalAdcStatus_t MockTryRead(void *context, HalAdcSample_t *sample)
{
    MockAdcDriver_t *driver = (MockAdcDriver_t *)context;
    if (driver->read_status == HAL_ADC_STATUS_OK)
    {
        sample->raw_code = driver->raw_code;
        sample->channel = driver->channel;
    }
    return driver->read_status;
}

int main(void)
{
    static const HalAdcDriverOps_t ops = {MockInitialize, MockTryRead};
    MockAdcDriver_t drivers[2] =
    {
        {HAL_ADC_STATUS_OK, HAL_ADC_STATUS_OK, 0x123456UL, 3U, 0U},
        {HAL_ADC_STATUS_IO_ERROR, HAL_ADC_STATUS_NOT_READY, 0U, 0U, 0U}
    };
    AnalogInputSample_t sample;
    AnalogInputDiagnostics_t diagnostics;

    assert(HalAdc_RegisterDriver(0U, &ops, &drivers[0]) ==
           HAL_ADC_STATUS_OK);
    assert(HalAdc_RegisterDriver(1U, &ops, &drivers[1]) ==
           HAL_ADC_STATUS_OK);
    assert(AnalogInputService_Initialize(2U) == ANALOG_INPUT_STATUS_OK);
    assert(drivers[0].initialize_count == 1U);
    assert(drivers[1].initialize_count == 1U);

    assert(AnalogInputService_Process() == ANALOG_INPUT_STATUS_OK);
    assert(AnalogInputService_GetLatest(0U, &sample));
    assert(sample.raw_code == 0x123456UL);
    assert(sample.channel == 3U);
    assert(sample.sequence == 1U);

    assert(AnalogInputService_Process() == ANALOG_INPUT_STATUS_NOT_READY);
    assert(AnalogInputService_GetDiagnostics(1U, &diagnostics));
    assert(!diagnostics.online);
    assert(diagnostics.driver_errors == 1U);

    drivers[1].initialize_status = HAL_ADC_STATUS_OK;
    drivers[1].read_status = HAL_ADC_STATUS_NOT_READY;
    assert(AnalogInputService_RetryDevice(1U) == ANALOG_INPUT_STATUS_OK);
    assert(AnalogInputService_Process() == ANALOG_INPUT_STATUS_OK);
    assert(AnalogInputService_Process() == ANALOG_INPUT_STATUS_NOT_READY);
    assert(AnalogInputService_GetDiagnostics(1U, &diagnostics));
    assert(diagnostics.online);
    assert(diagnostics.not_ready_polls == 1U);
    return 0;
}
