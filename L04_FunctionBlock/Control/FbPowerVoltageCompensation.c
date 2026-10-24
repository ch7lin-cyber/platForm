#include "FbPowerVoltageCompensation.h"
#include <stddef.h>

void FbPowerVoltageCompensation_DefaultConfig(FbPowerVoltageCompensationConfig *c)
{
    if (c!=NULL) *c=(FbPowerVoltageCompensationConfig){FB_VOLTAGE_BYPASS,0,1000,100,250,4000,1000};
}
bool FbPowerVoltageCompensation_ValidateConfig(const FbPowerVoltageCompensationConfig *c)
{
    return c!=NULL && c->maximum_mv<=1000 &&
        (c->mode==FB_VOLTAGE_BYPASS ||
         (c->mode==FB_VOLTAGE_SQUARED && c->nominal_mv>0 && c->minimum_voltage_mv>0 &&
          c->minimum_gain_permille>=1 && c->minimum_gain_permille<=c->maximum_gain_permille &&
          c->maximum_gain_permille<=10000));
}
FbVoltageCompensationStatus FbPowerVoltageCompensation_Execute(
    const FbPowerVoltageCompensationConfig *c,
    const FbPowerVoltageCompensationInput *in, FbPowerVoltageCompensationOutput *o)
{
    double candidate;
    if (o==NULL) return FB_VOLTAGE_INVALID_ARGUMENT;
    *o=(FbPowerVoltageCompensationOutput){0}; o->status=FB_VOLTAGE_INVALID_ARGUMENT;
    if (c==NULL || in==NULL) return o->status;
    if (!in->enable) { o->status=FB_VOLTAGE_DISABLED; return o->status; }
    if (!FbPowerVoltageCompensation_ValidateConfig(c)) {
        o->status=FB_VOLTAGE_INVALID_CONFIG; return o->status;
    }
    if (!in->mv_valid || in->mv<0 || in->mv>1000) {
        o->status=FB_VOLTAGE_INVALID_INPUT; return o->status;
    }
    o->bypassed=c->mode==FB_VOLTAGE_BYPASS;
    if (o->bypassed) o->raw_gain=o->applied_gain=1.0;
    else {
        double ratio, low, high;
        if (!in->voltage_valid) { o->status=FB_VOLTAGE_INVALID_INPUT; return o->status; }
        if (c->maximum_age_ms!=0 && in->voltage_age_ms>c->maximum_age_ms) {
            o->status=FB_VOLTAGE_STALE; return o->status;
        }
        if (in->voltage_mv<c->minimum_voltage_mv) {
            o->status=FB_VOLTAGE_TOO_LOW; return o->status;
        }
        ratio=(double)c->nominal_mv/in->voltage_mv;
        o->raw_gain=ratio*ratio; o->applied_gain=o->raw_gain;
        low=(double)c->minimum_gain_permille/1000;
        high=(double)c->maximum_gain_permille/1000;
        if (o->applied_gain<low) { o->applied_gain=low; o->gain_limited=true; }
        if (o->applied_gain>high) { o->applied_gain=high; o->gain_limited=true; }
    }
    o->requested_mv=(double)in->mv*o->applied_gain;
    candidate=o->requested_mv;
    if (candidate>c->maximum_mv) { candidate=c->maximum_mv; o->output_limited=true; }
    o->mv=(int32_t)(candidate+0.5); /* nonnegative, round ties upward */
    o->at_limit=o->mv==c->maximum_mv;
    o->valid=true; o->status=FB_VOLTAGE_OK;
    return o->status;
}
