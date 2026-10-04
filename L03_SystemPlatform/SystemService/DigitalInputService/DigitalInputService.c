#include "DigitalInputService.h"

#include <stddef.h>
#include <string.h>

#include "HalGpio.h"

static DigitalInputSnapshot_t g_snapshot;
static uint8_t g_channel_count;
static bool g_initialized;

static DigitalInputStatus_t SampleInputs(bool record_changes)
{
    uint16_t new_mask = 0U;
    uint8_t channel;

    for (channel = 0U; channel < g_channel_count; channel++)
    {
        bool active;
        if (HalGpio_ReadInput(channel, &active) != HAL_GPIO_STATUS_OK)
        {
            return DIGITAL_INPUT_STATUS_DRIVER_ERROR;
        }
        if (active)
        {
            new_mask |= (uint16_t)(1UL << channel);
        }
    }

    if (record_changes)
    {
        uint16_t changed = (uint16_t)(g_snapshot.state_mask ^ new_mask);
        if (changed != 0U)
        {
            g_snapshot.changed_mask |= changed;
            g_snapshot.revision++;
            if (g_snapshot.revision == 0U)
            {
                g_snapshot.revision = 1U;
            }
        }
    }
    g_snapshot.state_mask = new_mask;
    return DIGITAL_INPUT_STATUS_OK;
}

DigitalInputStatus_t DigitalInputService_Initialize(uint8_t channel_count)
{
    uint8_t channel;

    if ((channel_count == 0U) ||
        (channel_count > HAL_GPIO_INPUT_CHANNEL_COUNT))
    {
        return DIGITAL_INPUT_STATUS_INVALID_ARGUMENT;
    }
    (void)memset(&g_snapshot, 0, sizeof(g_snapshot));
    g_channel_count = channel_count;
    g_snapshot.valid_mask =
        (uint16_t)((1UL << channel_count) - 1UL);
    g_initialized = false;
    for (channel = 0U; channel < channel_count; channel++)
    {
        if (HalGpio_InitializeInput(channel) != HAL_GPIO_STATUS_OK)
        {
            return DIGITAL_INPUT_STATUS_DRIVER_ERROR;
        }
    }
    if (SampleInputs(false) != DIGITAL_INPUT_STATUS_OK)
    {
        return DIGITAL_INPUT_STATUS_DRIVER_ERROR;
    }
    g_initialized = true;
    return DIGITAL_INPUT_STATUS_OK;
}

DigitalInputStatus_t DigitalInputService_Process(void)
{
    if (!g_initialized)
    {
        return DIGITAL_INPUT_STATUS_NOT_INITIALIZED;
    }
    return SampleInputs(true);
}

bool DigitalInputService_GetState(uint8_t channel, bool *active)
{
    if (!g_initialized || (channel >= g_channel_count) || (active == NULL))
    {
        return false;
    }
    *active = (g_snapshot.state_mask & (uint16_t)(1UL << channel)) != 0U;
    return true;
}

bool DigitalInputService_GetSnapshot(DigitalInputSnapshot_t *snapshot)
{
    if (!g_initialized || (snapshot == NULL))
    {
        return false;
    }
    *snapshot = g_snapshot;
    return true;
}

void DigitalInputService_ClearChangedMask(uint16_t mask)
{
    if (g_initialized)
    {
        g_snapshot.changed_mask &= (uint16_t)~mask;
    }
}
