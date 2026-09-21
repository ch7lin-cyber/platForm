#include "AnalogInputService.h"

#include <stddef.h>
#include <string.h>

static AnalogInputSample_t g_latest[HAL_ADC_DEVICE_COUNT];
static AnalogInputDiagnostics_t g_diagnostics[HAL_ADC_DEVICE_COUNT];
static uint8_t g_device_count;
static uint8_t g_next_device;
static bool g_initialized;
static const HalAdcDeviceConfig_t *g_device_configs[HAL_ADC_DEVICE_COUNT];
static AnalogInputRoute_t g_routes[HAL_ADC_DEVICE_COUNT *
                                    HAL_ADC_CHANNELS_PER_DEVICE];
static AnalogInputSample_t g_input_latest[HAL_ADC_DEVICE_COUNT *
                                           HAL_ADC_CHANNELS_PER_DEVICE];
static uint8_t g_route_count;

AnalogInputStatus_t AnalogInputService_SetDeviceConfiguration(
    uint8_t device, const HalAdcDeviceConfig_t *config)
{
    if ((device >= HAL_ADC_DEVICE_COUNT) || (config == NULL) || g_initialized)
    {
        return ANALOG_INPUT_STATUS_INVALID_ARGUMENT;
    }
    g_device_configs[device] = config;
    return ANALOG_INPUT_STATUS_OK;
}

AnalogInputStatus_t AnalogInputService_SetRoutes(
    const AnalogInputRoute_t *routes, uint8_t route_count)
{
    uint8_t index;
    uint8_t previous;

    if ((routes == NULL) || (route_count == 0U) ||
        (route_count > (HAL_ADC_DEVICE_COUNT * HAL_ADC_CHANNELS_PER_DEVICE)) ||
        g_initialized)
    {
        return ANALOG_INPUT_STATUS_INVALID_ARGUMENT;
    }
    for (index = 0U; index < route_count; index++)
    {
        if ((routes[index].device >= HAL_ADC_DEVICE_COUNT) ||
            (routes[index].channel >= HAL_ADC_CHANNELS_PER_DEVICE) ||
            (routes[index].logical_input >=
             (HAL_ADC_DEVICE_COUNT * HAL_ADC_CHANNELS_PER_DEVICE)) ||
            (routes[index].sensor_class == ANALOG_INPUT_SENSOR_DISABLED))
        {
            return ANALOG_INPUT_STATUS_INVALID_ARGUMENT;
        }
        for (previous = 0U; previous < index; previous++)
        {
            if ((routes[previous].logical_input == routes[index].logical_input) ||
                ((routes[previous].device == routes[index].device) &&
                 (routes[previous].channel == routes[index].channel)))
            {
                return ANALOG_INPUT_STATUS_INVALID_ARGUMENT;
            }
        }
    }
    (void)memcpy(g_routes, routes, route_count * sizeof(g_routes[0]));
    g_route_count = route_count;
    return ANALOG_INPUT_STATUS_OK;
}

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
    (void)memset(g_input_latest, 0, sizeof(g_input_latest));
    g_device_count = device_count;
    g_next_device = 0U;
    for (device = 0U; device < device_count; device++)
    {
        HalAdcStatus_t status = HalAdc_Initialize(device);
        if ((status == HAL_ADC_STATUS_OK) &&
            (g_device_configs[device] != NULL))
        {
            status = HalAdc_Configure(device, g_device_configs[device]);
        }
        if (status == HAL_ADC_STATUS_OK)
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
    {
        uint8_t route_index;
        for (route_index = 0U; route_index < g_route_count; route_index++)
        {
            if ((g_routes[route_index].device == device) &&
                (g_routes[route_index].channel == hal_sample.channel))
            {
                AnalogInputSample_t *input =
                    &g_input_latest[g_routes[route_index].logical_input];
                input->raw_code = hal_sample.raw_code;
                input->channel = hal_sample.channel;
                input->sequence++;
                input->valid = true;
                break;
            }
        }
    }
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
    if ((status == HAL_ADC_STATUS_OK) && (g_device_configs[device] != NULL))
    {
        status = HalAdc_Configure(device, g_device_configs[device]);
    }
    g_diagnostics[device].online = (status == HAL_ADC_STATUS_OK);
    if (status != HAL_ADC_STATUS_OK)
    {
        g_diagnostics[device].driver_errors++;
    }
    return MapHalStatus(status);
}

bool AnalogInputService_GetLatestByInput(uint8_t logical_input,
                                         AnalogInputSample_t *sample)
{
    if ((!g_initialized) || (sample == NULL) ||
        (logical_input >= (HAL_ADC_DEVICE_COUNT * HAL_ADC_CHANNELS_PER_DEVICE)) ||
        !g_input_latest[logical_input].valid)
    {
        return false;
    }
    *sample = g_input_latest[logical_input];
    return true;
}

bool AnalogInputService_GetRoute(uint8_t logical_input,
                                 AnalogInputRoute_t *route)
{
    uint8_t index;

    if (route == NULL)
    {
        return false;
    }
    for (index = 0U; index < g_route_count; index++)
    {
        if (g_routes[index].logical_input == logical_input)
        {
            *route = g_routes[index];
            return true;
        }
    }
    return false;
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
