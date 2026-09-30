#ifndef FACTORY_MODE_SERVICE_H
#define FACTORY_MODE_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

#define FACTORY_MODE_UNLOCK_KEY (0x1234U)

void FactoryModeService_Initialize(void);
void FactoryModeService_SetUnlockKey1(uint16_t key);
void FactoryModeService_SetUnlockKey2(uint16_t key);
bool FactoryModeService_IsActive(void);
void FactoryModeService_Lock(void);

#endif
