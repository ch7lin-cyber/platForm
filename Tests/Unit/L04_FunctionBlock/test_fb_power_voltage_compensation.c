#include "FbPowerVoltageCompensation.h"
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
int main(void)
{
    FbPowerVoltageCompensationConfig c;
    FbPowerVoltageCompensationInput in={400,230000,0,true,true,true};
    FbPowerVoltageCompensationOutput o;
#define RUN() FbPowerVoltageCompensation_Execute(&c,&in,&o)
    FbPowerVoltageCompensation_DefaultConfig(&c);
    assert(FbPowerVoltageCompensation_ValidateConfig(&c));
    in.voltage_valid=false; assert(RUN()==FB_VOLTAGE_OK && o.bypassed && o.mv==400);
    c.mode=FB_VOLTAGE_SQUARED; c.nominal_mv=230000; in.voltage_valid=true;
    RUN(); assert(o.mv==400 && o.raw_gain==1 && !o.gain_limited);
    in.voltage_mv=115000; RUN(); assert(o.applied_gain==4 && o.mv==1000 && o.output_limited);
    in.mv=100; RUN(); assert(o.mv==400 && !o.output_limited);
    in.voltage_mv=460000; RUN(); assert(o.applied_gain==0.25 && o.mv==25);
    in.voltage_mv=1000000; RUN(); assert(o.gain_limited && o.applied_gain==0.25 && o.mv==25);
    in.voltage_mv=1000; RUN(); assert(o.gain_limited && o.applied_gain==4 && o.mv==400);
    in.mv=0; RUN(); assert(o.mv==0 && o.valid); /* no unwanted minimum power */
    in.mv=100; c.maximum_mv=123; RUN(); assert(o.mv==123 && o.output_limited && o.at_limit);
    c.maximum_mv=0; RUN(); assert(o.mv==0 && o.output_limited);
    c.maximum_mv=1000; in.voltage_mv=0;
    assert(RUN()==FB_VOLTAGE_TOO_LOW && !o.valid && o.mv==0);
    in.voltage_mv=230000; in.voltage_age_ms=100; assert(RUN()==FB_VOLTAGE_OK);
    in.voltage_age_ms=101; assert(RUN()==FB_VOLTAGE_STALE);
    c.maximum_age_ms=0; in.voltage_age_ms=UINT32_MAX; assert(RUN()==FB_VOLTAGE_OK);
    in.voltage_valid=false; assert(RUN()==FB_VOLTAGE_INVALID_INPUT);
    in.voltage_valid=true; in.mv_valid=false; assert(RUN()==FB_VOLTAGE_INVALID_INPUT);
    in.mv_valid=true; in.mv=-1; assert(RUN()==FB_VOLTAGE_INVALID_INPUT);
    in.mv=1001; assert(RUN()==FB_VOLTAGE_INVALID_INPUT);
    /* Maximum voltage ratio remains finite and bounded before MV conversion. */
    in.mv=1000; c.nominal_mv=UINT32_MAX; c.minimum_voltage_mv=1; in.voltage_mv=1;
    RUN(); assert(isfinite(o.raw_gain) && o.applied_gain==4 && o.mv==1000);
    c.nominal_mv=1; in.voltage_mv=UINT32_MAX;
    RUN(); assert(o.applied_gain==0.25 && o.mv==250);
    c.nominal_mv=1000; in.voltage_mv=1000; in.mv=1;
    c.minimum_gain_permille=c.maximum_gain_permille=1500;
    RUN(); assert(o.mv==2); /* 1.5 -> ties upward */
    c.minimum_gain_permille=0; assert(RUN()==FB_VOLTAGE_INVALID_CONFIG);
    c.minimum_gain_permille=2000; c.maximum_gain_permille=1000;
    assert(RUN()==FB_VOLTAGE_INVALID_CONFIG);
    in.enable=false; assert(RUN()==FB_VOLTAGE_DISABLED && !o.valid && o.mv==0);
    assert(FbPowerVoltageCompensation_Execute(NULL,&in,&o)==FB_VOLTAGE_INVALID_ARGUMENT);
    assert(FbPowerVoltageCompensation_Execute(&c,NULL,&o)==FB_VOLTAGE_INVALID_ARGUMENT);
    assert(FbPowerVoltageCompensation_Execute(&c,&in,NULL)==FB_VOLTAGE_INVALID_ARGUMENT);
    assert(!FbPowerVoltageCompensation_ValidateConfig(NULL));
    FbPowerVoltageCompensation_DefaultConfig(NULL);
    puts("FbPowerVoltageCompensation: squared gain, clamps, bypass, quality and range PASS");
    return 0;
}
