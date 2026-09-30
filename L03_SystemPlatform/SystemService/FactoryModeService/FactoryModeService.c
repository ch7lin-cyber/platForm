#include "FactoryModeService.h"

static uint16_t g_unlock_key1;
static uint16_t g_unlock_key2;

void FactoryModeService_Initialize(void)
{
    g_unlock_key1 = 0U;
    g_unlock_key2 = 0U;
}

void FactoryModeService_SetUnlockKey1(uint16_t key)
{
    g_unlock_key1 = key;
}

void FactoryModeService_SetUnlockKey2(uint16_t key)
{
    g_unlock_key2 = key;
}

bool FactoryModeService_IsActive(void)
{
    return (g_unlock_key1 == FACTORY_MODE_UNLOCK_KEY) &&
           (g_unlock_key2 == FACTORY_MODE_UNLOCK_KEY);
}

void FactoryModeService_Lock(void)
{
    g_unlock_key1 = 0U;
    g_unlock_key2 = 0U;
}
