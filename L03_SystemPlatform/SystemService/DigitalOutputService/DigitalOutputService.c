#include "DigitalOutputService.h"

#include <stddef.h>
#include <string.h>

#include "HalGpio.h"

static DigitalOutputState_t g_state;
static uint8_t g_channel_count;
static bool g_initialized;

DigitalOutputStatus_t DigitalOutputService_Initialize(uint8_t channel_count)
{
    uint8_t channel;

    if ((channel_count == 0U) ||
        (channel_count > HAL_GPIO_OUTPUT_CHANNEL_COUNT))
    {
        return DIGITAL_OUTPUT_STATUS_INVALID_ARGUMENT;
    }
    (void)memset(&g_state, 0, sizeof(g_state));
    g_channel_count = channel_count;
    g_state.valid_mask = (uint16_t)((1UL << channel_count) - 1UL);
    g_state.failed_channel = DIGITAL_OUTPUT_FAILED_CHANNEL_NONE;
    g_initialized = false;
    for (channel = 0U; channel < channel_count; channel++)
    {
        if ((HalGpio_InitializeOutput(channel) != HAL_GPIO_STATUS_OK) ||
            (HalGpio_WriteOutput(channel, false) != HAL_GPIO_STATUS_OK))
        {
            g_state.last_status = DIGITAL_OUTPUT_STATUS_DRIVER_ERROR;
            g_state.failed_channel = channel;
            return DIGITAL_OUTPUT_STATUS_DRIVER_ERROR;
        }
    }
    g_state.last_status = DIGITAL_OUTPUT_STATUS_OK;
    g_initialized = true;
    return DIGITAL_OUTPUT_STATUS_OK;
}

DigitalOutputStatus_t DigitalOutputService_SetChannel(uint8_t channel,
                                                       bool active)
{
    uint16_t mask;

    if (channel >= g_channel_count)
    {
        return DIGITAL_OUTPUT_STATUS_INVALID_ARGUMENT;
    }
    if (!g_initialized)
    {
        return DIGITAL_OUTPUT_STATUS_NOT_INITIALIZED;
    }
    mask = g_state.active_mask;
    if (active)
    {
        mask |= (uint16_t)(1UL << channel);
    }
    else
    {
        mask &= (uint16_t)~(uint16_t)(1UL << channel);
    }
    return DigitalOutputService_SetMask(mask);
}

DigitalOutputStatus_t DigitalOutputService_SetMask(uint16_t active_mask)
{
    uint16_t changed_mask;
    uint8_t channel;

    if (!g_initialized)
    {
        return DIGITAL_OUTPUT_STATUS_NOT_INITIALIZED;
    }
    if ((active_mask & (uint16_t)~g_state.valid_mask) != 0U)
    {
        return DIGITAL_OUTPUT_STATUS_INVALID_ARGUMENT;
    }
    changed_mask = (uint16_t)(g_state.active_mask ^ active_mask);
    if (changed_mask == 0U)
    {
        g_state.last_status = DIGITAL_OUTPUT_STATUS_OK;
        g_state.failed_channel = DIGITAL_OUTPUT_FAILED_CHANNEL_NONE;
        return DIGITAL_OUTPUT_STATUS_OK;
    }

    for (channel = 0U; channel < g_channel_count; channel++)
    {
        uint16_t channel_mask = (uint16_t)(1UL << channel);
        if ((changed_mask & channel_mask) != 0U)
        {
            bool requested = (active_mask & channel_mask) != 0U;
            if (HalGpio_WriteOutput(channel, requested) !=
                HAL_GPIO_STATUS_OK)
            {
                uint8_t rollback_channel;
                bool rollback_failed = false;

                g_state.last_status = DIGITAL_OUTPUT_STATUS_DRIVER_ERROR;
                g_state.failed_channel = channel;
                for (rollback_channel = 0U; rollback_channel < channel;
                     rollback_channel++)
                {
                    uint16_t rollback_mask =
                        (uint16_t)(1UL << rollback_channel);
                    if ((changed_mask & rollback_mask) != 0U)
                    {
                        bool old_state =
                            (g_state.active_mask & rollback_mask) != 0U;
                        if (HalGpio_WriteOutput(rollback_channel, old_state) !=
                            HAL_GPIO_STATUS_OK)
                        {
                            rollback_failed = true;
                        }
                    }
                }
                if (rollback_failed)
                {
                    g_state.last_status =
                        DIGITAL_OUTPUT_STATUS_ROLLBACK_ERROR;
                }
                return g_state.last_status;
            }
        }
    }

    g_state.active_mask = active_mask;
    g_state.revision++;
    if (g_state.revision == 0U)
    {
        g_state.revision = 1U;
    }
    g_state.last_status = DIGITAL_OUTPUT_STATUS_OK;
    g_state.failed_channel = DIGITAL_OUTPUT_FAILED_CHANNEL_NONE;
    return DIGITAL_OUTPUT_STATUS_OK;
}

bool DigitalOutputService_GetState(DigitalOutputState_t *state)
{
    if (!g_initialized || (state == NULL))
    {
        return false;
    }
    *state = g_state;
    return true;
}
