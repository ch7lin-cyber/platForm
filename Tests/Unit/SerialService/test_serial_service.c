#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "HalSerial.h"
#include "SerialConfiguration.h"
#include "SerialService.h"

typedef struct
{
    bool busy;
    HalSerialConfig_t configuration;
    uint8_t transmitted[SERIAL_SERVICE_TX_CAPACITY];
    size_t transmitted_length;
} FakeDriver_t;

static uint32_t g_events[HAL_SERIAL_PORT_COUNT];

static HalSerialStatus_t FakeInitialize(void *context,
                                        const HalSerialConfig_t *configuration)
{
    FakeDriver_t *driver = (FakeDriver_t *)context;
    driver->configuration = *configuration;
    driver->busy = false;
    return HAL_SERIAL_STATUS_OK;
}

static HalSerialStatus_t FakeConfigure(void *context,
                                       const HalSerialConfig_t *configuration)
{
    FakeDriver_t *driver = (FakeDriver_t *)context;
    driver->configuration = *configuration;
    return HAL_SERIAL_STATUS_OK;
}

static HalSerialStatus_t FakeWrite(void *context,
                                   const uint8_t *data,
                                   size_t length)
{
    FakeDriver_t *driver = (FakeDriver_t *)context;
    (void)memcpy(driver->transmitted, data, length);
    driver->transmitted_length = length;
    driver->busy = true;
    return HAL_SERIAL_STATUS_OK;
}

static HalSerialStatus_t FakeAbort(void *context)
{
    ((FakeDriver_t *)context)->busy = false;
    return HAL_SERIAL_STATUS_OK;
}

static bool FakeIsBusy(void *context)
{
    return ((FakeDriver_t *)context)->busy;
}

static void OnEvent(HalSerialPort_t port, uint32_t event_mask, void *context)
{
    (void)context;
    g_events[(uint32_t)port] |= event_mask;
}

int main(void)
{
    static const HalSerialDriverOps_t operations = {
        FakeInitialize, FakeConfigure, FakeWrite, FakeAbort, FakeIsBusy
    };
    FakeDriver_t drivers[HAL_SERIAL_PORT_COUNT] = {0};
    SerialConfiguration_t configuration;
    SerialServiceStatistics_t statistics;
    uint8_t receive_data[] = {0x01U, 0x03U, 0x00U, 0x00U};
    uint8_t read_data[sizeof(receive_data)] = {0};
    uint8_t transmit_data[] = {0x02U, 0x06U, 0x11U, 0x03U};
    uint32_t index;

    for (index = 0U; index < HAL_SERIAL_PORT_COUNT; index++)
    {
        assert(HalSerial_RegisterDriver((HalSerialPort_t)index,
                                        &operations, &drivers[index]) ==
               HAL_SERIAL_STATUS_OK);
    }

    SerialConfiguration_SetRtuDefault(&configuration, SERIAL_ROLE_MODBUS_MASTER);
    assert(SerialConfiguration_IsValid(&configuration));
    assert(SerialService_InitializePort(HAL_SERIAL_PORT_0, &configuration,
                                        OnEvent, NULL) ==
           SERIAL_SERVICE_STATUS_OK);

    SerialConfiguration_SetAsciiDefault(&configuration, SERIAL_ROLE_MODBUS_SLAVE);
    assert(SerialConfiguration_IsValid(&configuration));
    assert(SerialService_InitializePort(HAL_SERIAL_PORT_1, &configuration,
                                        OnEvent, NULL) ==
           SERIAL_SERVICE_STATUS_OK);

    HalSerial_NotifyReceiveFromIsr(HAL_SERIAL_PORT_0,
                                   receive_data, sizeof(receive_data));
    SerialService_Tick1ms();
    SerialService_Tick1ms();
    SerialService_Process();
    assert((g_events[0] & SERIAL_SERVICE_EVENT_RX_AVAILABLE) != 0U);
    assert((g_events[0] & SERIAL_SERVICE_EVENT_RX_IDLE) != 0U);
    assert(SerialService_Read(HAL_SERIAL_PORT_0, read_data,
                              sizeof(read_data)) == sizeof(read_data));
    assert(memcmp(receive_data, read_data, sizeof(read_data)) == 0);

    assert(SerialService_Write(HAL_SERIAL_PORT_0, transmit_data,
                               sizeof(transmit_data)) == SERIAL_SERVICE_STATUS_OK);
    assert(SerialService_Write(HAL_SERIAL_PORT_0, transmit_data,
                               sizeof(transmit_data)) == SERIAL_SERVICE_STATUS_BUSY);
    drivers[0].busy = false;
    HalSerial_NotifyTransmitCompleteFromIsr(HAL_SERIAL_PORT_0);
    SerialService_Process();
    assert((g_events[0] & SERIAL_SERVICE_EVENT_TX_COMPLETE) != 0U);

    assert(SerialService_GetStatistics(HAL_SERIAL_PORT_0, &statistics));
    assert(statistics.received_bytes == sizeof(receive_data));
    assert(statistics.transmitted_bytes == sizeof(transmit_data));
    assert(!statistics.transmit_busy);
    return 0;
}
