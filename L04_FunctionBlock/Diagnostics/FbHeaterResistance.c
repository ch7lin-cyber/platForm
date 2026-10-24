#include "FbHeaterResistance.h"
#include <stddef.h>

void FbHeaterResistance_DefaultConfig(FbHeaterResistanceConfig *c)
{
    if (c!=NULL) *c=(FbHeaterResistanceConfig){100,10,100,10,false,0,0,false,0};
}
bool FbHeaterResistance_ValidateConfig(const FbHeaterResistanceConfig *c)
{
    return c!=NULL && c->minimum_voltage_mv>0 && c->minimum_current_ma>0 &&
        (!c->limits_enabled || (c->low_mohm<=c->high_mohm &&
         c->high_mohm<=FB_HEATER_RESISTANCE_MAX_MOHM)) &&
        (!c->reference_enabled || (c->reference_mohm>0 &&
         c->reference_mohm<=FB_HEATER_RESISTANCE_MAX_MOHM));
}
FbHeaterResistanceStatus FbHeaterResistance_Execute(
    const FbHeaterResistanceConfig *c, const FbHeaterResistanceInput *in,
    FbHeaterResistanceOutput *o)
{
    uint32_t skew;
    if (o==NULL) return FB_HEATER_INVALID_ARGUMENT;
    *o=(FbHeaterResistanceOutput){0}; o->status=FB_HEATER_INVALID_ARGUMENT;
    if (c==NULL || in==NULL) return o->status;
    if (!in->enable) { o->status=FB_HEATER_DISABLED; return o->status; }
    if (!in->energized) { o->status=FB_HEATER_INACTIVE; return o->status; }
    if (!FbHeaterResistance_ValidateConfig(c)) {
        o->status=FB_HEATER_INVALID_CONFIG; return o->status;
    }
    if (!in->voltage_valid || !in->current_valid) {
        o->status=FB_HEATER_INVALID_INPUT; return o->status;
    }
    if (c->maximum_age_ms!=0 && (in->voltage_age_ms>c->maximum_age_ms ||
        in->current_age_ms>c->maximum_age_ms)) { o->status=FB_HEATER_STALE; return o->status; }
    skew=in->voltage_age_ms>=in->current_age_ms ?
        in->voltage_age_ms-in->current_age_ms : in->current_age_ms-in->voltage_age_ms;
    if (skew>c->maximum_skew_ms) { o->status=FB_HEATER_UNALIGNED; return o->status; }
    if (in->voltage_mv<c->minimum_voltage_mv) {
        o->status=FB_HEATER_LOW_VOLTAGE; return o->status;
    }
    if (in->current_ma<c->minimum_current_ma) {
        o->status=FB_HEATER_LOW_CURRENT; return o->status;
    }
    /* Round nonnegative ratios to nearest, exact ties upward. */
    o->resistance_mohm=((uint64_t)in->voltage_mv*1000+in->current_ma/2)/in->current_ma;
    o->apparent_power_mva=((uint64_t)in->voltage_mv*in->current_ma+500)/1000;
    if (c->limits_enabled) {
        o->below_limit=o->resistance_mohm<c->low_mohm;
        o->above_limit=o->resistance_mohm>c->high_mohm;
    }
    if (c->reference_enabled) {
        int64_t delta=(int64_t)o->resistance_mohm-(int64_t)c->reference_mohm;
        uint64_t magnitude=(uint64_t)(delta<0 ? -delta : delta);
        int64_t ratio=(int64_t)((magnitude*1000+c->reference_mohm/2)/c->reference_mohm);
        o->reference_change_permille=delta<0 ? -ratio : ratio;
        o->reference_valid=true;
    }
    o->valid=true; o->status=FB_HEATER_OK;
    return o->status;
}
