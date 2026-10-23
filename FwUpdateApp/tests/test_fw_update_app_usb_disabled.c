#include "../include/fw_update_app.h"
#include <assert.h>

static bool Quiesce(void *user) { (void)user; return true; }
static void Reset(void *user, uint16_t route, uint16_t unit, uint32_t baud)
{
    (void)user;
    (void)route;
    (void)unit;
    (void)baud;
}

int main(void)
{
    const FwUpdateAppConfig_t config = {
        .allowed_routes = FW_UPDATE_APP_ROUTE_USB | FW_UPDATE_APP_ROUTE_RS485,
        .reply_delay_ms = 200U
    };
    const FwUpdateAppPlatform_t platform = {
        .user = 0,
        .quiesce = Quiesce,
        .reset = Reset
    };
    FwUpdateApp_t app;

    assert(FwUpdateApp_Init(&app, &config, &platform));
    FwUpdateApp_SetRequestContext(&app, BOOT_REQUEST_USB, 2U);
    assert(FwUpdateApp_Write(&app, BOOT_REQUEST_REGISTER,
                             BOOT_REQUEST_USB_KEY) == 3U);
    assert(!FwUpdateApp_IsPending(&app));

    FwUpdateApp_SetRequestContext(&app, BOOT_REQUEST_RS485, 2U);
    assert(FwUpdateApp_Write(&app, BOOT_REQUEST_REGISTER,
                             BOOT_REQUEST_RS485_KEY) == 0U);
    assert(FwUpdateApp_IsPending(&app));
    return 0;
}
