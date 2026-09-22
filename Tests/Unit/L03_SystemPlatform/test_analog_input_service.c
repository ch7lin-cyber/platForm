#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
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
    uint32_t configure_count;
} MockAdcDriver_t;

static HalAdcStatus_t MockInitialize(void *context)
{
    MockAdcDriver_t *driver = (MockAdcDriver_t *)context;
    driver->initialize_count++;
    return driver->initialize_status;
}

static HalAdcStatus_t MockConfigure(void *context,
                                    const HalAdcDeviceConfig_t *config)
{
    MockAdcDriver_t *driver = (MockAdcDriver_t *)context;
    assert(config != NULL);
    driver->configure_count++;
    return HAL_ADC_STATUS_OK;
}

static HalAdcStatus_t MockTryRead(void *context, HalAdcSample_t *sample)
{
    MockAdcDriver_t *driver = (MockAdcDriver_t *)context;
    if (driver->read_status == HAL_ADC_STATUS_OK)
    {
        sample->raw_code = driver->raw_code;
        sample->microvolts = (int32_t)driver->raw_code;
        sample->channel = driver->channel;
    }
    return driver->read_status;
}

int main(void)
{
    static const HalAdcDriverOps_t ops =
        {MockInitialize, MockConfigure, MockTryRead};
    static const HalAdcSetupConfig_t setup =
        {HAL_ADC_REFERENCE_INTERNAL, HAL_ADC_FILTER_SINC4,
         HAL_ADC_GAIN_1, 384U, true, true, false};
    static const HalAdcChannelConfig_t channel =
        {3U, 0U, 0U, 1U, true};
    static const HalAdcDeviceConfig_t config =
        {&setup, 1U, &channel, 1U};
    static const AnalogInputRoute_t route =
        {0U, 0U, 3U, ANALOG_INPUT_SENSOR_THERMOCOUPLE};
    MockAdcDriver_t drivers[2] =
    {
        {HAL_ADC_STATUS_OK, HAL_ADC_STATUS_OK, 0x123456UL, 3U, 0U, 0U},
        {HAL_ADC_STATUS_IO_ERROR, HAL_ADC_STATUS_NOT_READY, 0U, 0U, 0U, 0U}
    };
    AnalogInputSample_t sample;
    AnalogInputDiagnostics_t diagnostics;

    assert(HalAdc_RegisterDriver(0U, &ops, &drivers[0]) ==
           HAL_ADC_STATUS_OK);
    assert(HalAdc_RegisterDriver(1U, &ops, &drivers[1]) ==
           HAL_ADC_STATUS_OK);
    assert(AnalogInputService_SetDeviceConfiguration(0U, &config) ==
           ANALOG_INPUT_STATUS_OK);
    assert(AnalogInputService_SetDeviceConfiguration(1U, &config) ==
           ANALOG_INPUT_STATUS_OK);
    assert(AnalogInputService_SetRoutes(&route, 1U) ==
           ANALOG_INPUT_STATUS_OK);
    assert(AnalogInputService_Initialize(2U) == ANALOG_INPUT_STATUS_OK);
    assert(drivers[0].initialize_count == 1U);
    assert(drivers[1].initialize_count == 1U);
    assert(drivers[0].configure_count == 1U);

    assert(AnalogInputService_Process() == ANALOG_INPUT_STATUS_OK);
    assert(AnalogInputService_GetLatest(0U, &sample));
    assert(sample.raw_code == 0x123456UL);
    assert(sample.channel == 3U);
    assert(sample.sequence == 1U);
    assert(AnalogInputService_GetLatestByInput(0U, &sample));
    assert(sample.raw_code == 0x123456UL);

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
