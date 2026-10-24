#include "FbHeaterResistance.h"
#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
int main(void)
{
    FbHeaterResistanceConfig c;
    FbHeaterResistanceInput in={230000,10000,10,20,true,true,true,true};
    FbHeaterResistanceOutput o;
#define RUN() FbHeaterResistance_Execute(&c,&in,&o)
    FbHeaterResistance_DefaultConfig(&c);
    assert(FbHeaterResistance_ValidateConfig(&c));
    assert(RUN()==FB_HEATER_OK && o.resistance_mohm==23000 && o.apparent_power_mva==2300000);
    c.reference_enabled=true; c.reference_mohm=20000;
    RUN(); assert(o.reference_valid && o.reference_change_permille==150);
    c.reference_mohm=25000; RUN(); assert(o.reference_change_permille==-80);
    c.limits_enabled=true; c.low_mohm=23000; c.high_mohm=23000;
    RUN(); assert(!o.below_limit && !o.above_limit);
    c.low_mohm=c.high_mohm=23001; RUN(); assert(o.below_limit && o.valid);
    c.low_mohm=c.high_mohm=22999; RUN(); assert(o.above_limit && o.valid);
    in.voltage_age_ms=100; in.current_age_ms=90; assert(RUN()==FB_HEATER_OK);
    in.voltage_age_ms=101; assert(RUN()==FB_HEATER_STALE && !o.valid && o.resistance_mohm==0);
    in.voltage_age_ms=100; in.current_age_ms=89; assert(RUN()==FB_HEATER_UNALIGNED);
    c.maximum_age_ms=0; c.maximum_skew_ms=UINT32_MAX;
    in.voltage_age_ms=UINT32_MAX; in.current_age_ms=0; assert(RUN()==FB_HEATER_OK);
    in.voltage_valid=false; assert(RUN()==FB_HEATER_INVALID_INPUT);
    in.voltage_valid=true; in.current_valid=false; assert(RUN()==FB_HEATER_INVALID_INPUT);
    in.current_valid=true; in.voltage_mv=0; in.current_ma=0;
    assert(RUN()==FB_HEATER_LOW_VOLTAGE);
    in.voltage_mv=230000; assert(RUN()==FB_HEATER_LOW_CURRENT);
    in.energized=false; assert(RUN()==FB_HEATER_INACTIVE && !o.valid);
    in.energized=true; c.minimum_voltage_mv=1; c.minimum_current_ma=1;
    c.limits_enabled=c.reference_enabled=false;
    /* 1 mV / 16mA = 62.5 milli-ohm, ties upward. */
    in.voltage_mv=1; in.current_ma=16;
    RUN(); assert(o.resistance_mohm==63);
    in.voltage_mv=1; in.current_ma=500;
    RUN(); assert(o.apparent_power_mva==1);
    /* Full uint32 products and largest possible resistance remain defined. */
    in.voltage_mv=UINT32_MAX; in.current_ma=1;
    c.reference_enabled=true; c.reference_mohm=1;
    RUN(); assert(o.resistance_mohm==FB_HEATER_RESISTANCE_MAX_MOHM);
    assert(o.reference_change_permille==INT64_C(4294967294999000));
    in.current_ma=UINT32_MAX; RUN();
    assert(o.resistance_mohm==1000 && o.apparent_power_mva==UINT64_C(18446744065119617));
    c.reference_mohm=0; assert(RUN()==FB_HEATER_INVALID_CONFIG);
    in.enable=false; assert(RUN()==FB_HEATER_DISABLED && !o.valid && !o.reference_valid);
    in.enable=true; c.reference_enabled=false; c.minimum_current_ma=0;
    assert(RUN()==FB_HEATER_INVALID_CONFIG);
    assert(FbHeaterResistance_Execute(NULL,&in,&o)==FB_HEATER_INVALID_ARGUMENT);
    assert(FbHeaterResistance_Execute(&c,NULL,&o)==FB_HEATER_INVALID_ARGUMENT);
    assert(FbHeaterResistance_Execute(&c,&in,NULL)==FB_HEATER_INVALID_ARGUMENT);
    assert(!FbHeaterResistance_ValidateConfig(NULL)); FbHeaterResistance_DefaultConfig(NULL);
    puts("FbHeaterResistance: units, ratio, reference, quality and integer boundaries PASS");
    return 0;
}
