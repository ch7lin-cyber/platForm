#include "AnalogInputService.h"

#include <stddef.h>
#include <string.h>

static AnalogInputSample_t g_latest[HAL_ADC_DEVICE_COUNT];
static AnalogInputDiagnostics_t g_diagnostics[HAL_ADC_DEVICE_COUNT];
static uint8_t g_device_count;
static uint8_t g_next_device;
static bool g_initialized;

static AnalogInputStatus_t MapHalStatus(HalAdcStatus_t status)
{
    if (status == HAL_ADC_STATUS_OK)
    {
        return ANALOG_INPUT_STATUS_OK;
    }
    if (status == HAL_ADC_STATUS_NOT_READY)
    {
        return ANALOG_INPUT_STATUS_NOT_READY;
    }
    if (status == HAL_ADC_STATUS_INVALID_ARGUMENT)
    {
        return ANALOG_INPUT_STATUS_INVALID_ARGUMENT;
    }
    return ANALOG_INPUT_STATUS_DRIVER_ERROR;
}

AnalogInputStatus_t AnalogInputService_Initialize(uint8_t device_count)
{
    uint8_t device;
    uint8_t online_count = 0U;

    if ((device_count == 0U) || (device_count > HAL_ADC_DEVICE_COUNT))
    {
        return ANALOG_INPUT_STATUS_INVALID_ARGUMENT;
    }

    (void)memset(g_latest, 0, sizeof(g_latest));
    (void)memset(g_diagnostics, 0, sizeof(g_diagnostics));
    g_device_count = device_count;
    g_next_device = 0U;
    for (device = 0U; device < device_count; device++)
    {
        if (HalAdc_Initialize(device) == HAL_ADC_STATUS_OK)
        {
            g_diagnostics[device].online = true;
            online_count++;
        }
        else
        {
            g_diagnostics[device].driver_errors++;
        }
    }

    g_initialized = true;
    return (online_count > 0U) ? ANALOG_INPUT_STATUS_OK :
                                ANALOG_INPUT_STATUS_DRIVER_ERROR;
}

AnalogInputStatus_t AnalogInputService_Process(void)
{
    HalAdcSample_t hal_sample;
    HalAdcStatus_t hal_status;
    AnalogInputDiagnostics_t *diagnostics;
    AnalogInputSample_t *latest;
    uint8_t device;

    if (!g_initialized)
    {
        return ANALOG_INPUT_STATUS_NOT_INITIALIZED;
    }

    device = g_next_device;
    g_next_device = (uint8_t)((g_next_device + 1U) % g_device_count);
    diagnostics = &g_diagnostics[device];
    if (!diagnostics->online)
    {
        return ANALOG_INPUT_STATUS_NOT_READY;
    }

    hal_status = HalAdc_TryRead(device, &hal_sample);
    if (hal_status == HAL_ADC_STATUS_NOT_READY)
    {
        diagnostics->not_ready_polls++;
        return ANALOG_INPUT_STATUS_NOT_READY;
    }
    if (hal_status != HAL_ADC_STATUS_OK)
    {
        diagnostics->driver_errors++;
        diagnostics->online = false;
        return MapHalStatus(hal_status);
    }

    latest = &g_latest[device];
    latest->raw_code = hal_sample.raw_code;
    latest->channel = hal_sample.channel;
    latest->sequence++;
    latest->valid = true;
    diagnostics->successful_samples++;
    return ANALOG_INPUT_STATUS_OK;
}

AnalogInputStatus_t AnalogInputService_RetryDevice(uint8_t device)
{
    HalAdcStatus_t status;

    if ((!g_initialized) || (device >= g_device_count))
    {
        return ANALOG_INPUT_STATUS_INVALID_ARGUMENT;
    }

    status = HalAdc_Initialize(device);
    g_diagnostics[device].online = (status == HAL_ADC_STATUS_OK);
    if (status != HAL_ADC_STATUS_OK)
    {
        g_diagnostics[device].driver_errors++;
    }
    return MapHalStatus(status);
}

bool AnalogInputService_GetLatest(uint8_t device,
                                  AnalogInputSample_t *sample)
{
    if ((!g_initialized) || (device >= g_device_count) ||
        (sample == NULL) || !g_latest[device].valid)
    {
        return false;
    }
    *sample = g_latest[device];
    return true;
}

bool AnalogInputService_GetDiagnostics(uint8_t device,
                                       AnalogInputDiagnostics_t *diagnostics)
{
    if ((!g_initialized) || (device >= g_device_count) ||
        (diagnostics == NULL))
    {
        return false;
    }
    *diagnostics = g_diagnostics[device];
    return true;
}
