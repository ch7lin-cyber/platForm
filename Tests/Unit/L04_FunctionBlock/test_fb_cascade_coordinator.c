#include "FbCascadeCoordinator.h"
#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
int main(void)
{
    FbCascadeCoordinator s={0}, other={0};
    FbCascadeConfig c;
    FbCascadeInput in={0,220000,true,true,100,true,false};
    FbCascadeOutput o;
#define RUN() FbCascadeCoordinator_Execute(&s,&c,&in,&o)
    FbCascadeCoordinator_DefaultConfig(&c);
    assert(FbCascadeCoordinator_ValidateConfig(&c));
    c.sv_low=-20000; c.sv_high=40000;
    assert(RUN()==FB_CASCADE_OK && o.slave_sv==200000 && o.at_low);
    in.master_mv=500; RUN(); assert(o.slave_sv==230000 && o.target_sv==230000);
    in.master_mv=1000; RUN(); assert(o.slave_sv==260000 && o.at_high);
    in.master_mv=-1; RUN(); assert(o.mv_clamped && o.slave_sv==200000);
    in.master_mv=1001; RUN(); assert(o.mv_clamped && o.slave_sv==260000);
    c.reverse_mapping=true; in.master_mv=0;
    RUN(); assert(o.slave_sv==260000);
    in.master_mv=1000; RUN(); assert(o.slave_sv==200000);
    c.reverse_mapping=false; c.mode=FB_CASCADE_ABSOLUTE;
    c.sv_low=0; c.sv_high=100000; in.master_sv_valid=false;
    in.master_mv=500; c.offset=1000;
    RUN(); assert(o.slave_sv==51000 && !o.target_clamped);
    in.master_mv=1000;
    RUN(); assert(o.slave_sv==100000 && o.target_clamped);
    in.master_mv=0; c.offset=-1000;
    RUN(); assert(o.slave_sv==0 && o.target_clamped);
    /* First valid sample seeds; backward-Euler alpha is dt/(tau+dt). */
    c.offset=0; c.filter_tau_ms=1000; in.elapsed_ms=1000;
    RUN(); assert(o.slave_sv==0);
    in.master_mv=1000;
    RUN(); assert(o.slave_sv==50000 && o.target_sv==100000);
    in.elapsed_ms=0; RUN(); assert(o.slave_sv==50000);
    in.elapsed_ms=1000; RUN(); assert(o.slave_sv==75000);
    FbCascadeCoordinator_Execute(&other,&c,&in,&o); assert(o.slave_sv==100000);
    /* Invalid input resets history, no old filtered value returned as valid. */
    in.mv_valid=false; assert(RUN()==FB_CASCADE_INVALID_INPUT && !o.valid && o.slave_sv==0);
    in.mv_valid=true; in.master_mv=0; RUN(); assert(o.slave_sv==0);
    c.offset=1234; RUN(); assert(o.slave_sv==1234); /* config change reseeds */
    /* Relative bounds move; final clamp always constrains old filter state. */
    c.offset=0; c.mode=FB_CASCADE_DEVIATION; c.sv_low=0; c.sv_high=1000;
    in.master_sv=0; in.master_sv_valid=true; in.master_mv=0; RUN();
    in.master_sv=10000; in.elapsed_ms=0;
    RUN(); assert(o.filter_clamped && o.slave_sv==10000 && o.at_low);
    in.master_sv=INT_MAX;
    assert(RUN()==FB_CASCADE_INVALID_INPUT);
    in.master_sv_valid=false; assert(RUN()==FB_CASCADE_INVALID_INPUT);
    /* Equal SV limits legal; full int32 range multiplication remains defined. */
    c.mode=FB_CASCADE_ABSOLUTE; c.sv_low=c.sv_high=-123;
    RUN(); assert(o.slave_sv==-123 && o.at_low && o.at_high);
    c.sv_low=INT_MIN; c.sv_high=INT_MAX; c.mv_low=INT_MIN; c.mv_high=INT_MAX;
    c.filter_tau_ms=0; in.master_mv=INT_MAX;
    RUN(); assert(o.slave_sv==INT_MAX);
    in.master_mv=INT_MIN; RUN(); assert(o.slave_sv==INT_MIN);
    c.reverse_mapping=true; RUN(); assert(o.slave_sv==INT_MAX);
    c.mv_low=c.mv_high;
    assert(RUN()==FB_CASCADE_INVALID_CONFIG && !o.valid);
    FbCascadeCoordinator_DefaultConfig(&c); c.mode=(FbCascadeMode)9;
    assert(RUN()==FB_CASCADE_INVALID_CONFIG);
    in.reset=true; assert(RUN()==FB_CASCADE_RESET && !s.seeded);
    in.enable=false; assert(RUN()==FB_CASCADE_DISABLED && !o.valid);
    assert(FbCascadeCoordinator_Execute(NULL,&c,&in,&o)==FB_CASCADE_INVALID_ARGUMENT);
    assert(FbCascadeCoordinator_Execute(&s,NULL,&in,&o)==FB_CASCADE_INVALID_ARGUMENT);
    assert(FbCascadeCoordinator_Execute(&s,&c,NULL,&o)==FB_CASCADE_INVALID_ARGUMENT);
    assert(FbCascadeCoordinator_Execute(&s,&c,&in,NULL)==FB_CASCADE_INVALID_ARGUMENT);
    assert(!FbCascadeCoordinator_ValidateConfig(NULL));
    FbCascadeCoordinator_DefaultConfig(NULL);
    puts("FbCascadeCoordinator: mapping, offset, limits, filter, quality and integer range PASS");
    return 0;
}
