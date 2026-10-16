#include "FbSetpointRamp.h"
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>

static unsigned cases;
#define CHECK(id, expression) do { assert(expression); ++cases; printf("%s PASS\n", id); } while (0)
int main(void)
{
    FbSetpointRampConfig c = {2.0F, 4.0F};
    FbSetpointRampInput i = {10.0F, 1.0F, true};
    FbSetpointRampState s = {0};
    FbSetpointRampState other = {0};
    FbSetpointRampOutput o;
    float saved;
    CHECK("UT-01", FbSetpointRamp_Init(&s, 0) == FB_SETPOINT_RAMP_OK && s.initialized && s.value == 0);
    CHECK("UT-02", FbSetpointRamp_Execute(&c,&i,&s,&o) == FB_SETPOINT_RAMP_OK && o.value == 2 && o.valid && !o.reached);
    i.dt_seconds=10;
    CHECK("UT-03", FbSetpointRamp_Execute(&c,&i,&s,&o) == FB_SETPOINT_RAMP_OK && o.value == 10 && o.reached);
    i.target=-10; i.dt_seconds=1;
    CHECK("UT-04", FbSetpointRamp_Execute(&c,&i,&s,&o) == FB_SETPOINT_RAMP_OK && o.value == 6 && !o.reached);
    i.dt_seconds=4;
    CHECK("UT-05", FbSetpointRamp_Execute(&c,&i,&s,&o) == FB_SETPOINT_RAMP_OK && o.value == -10 && o.reached);
    i.target=10; i.dt_seconds=0;
    CHECK("UT-06", FbSetpointRamp_Execute(&c,&i,&s,&o) == FB_SETPOINT_RAMP_OK && o.value == -10 && !o.reached);
    c.rise_rate=0; i.dt_seconds=5;
    CHECK("UT-07", FbSetpointRamp_Execute(&c,&i,&s,&o) == FB_SETPOINT_RAMP_OK && o.value == -10);
    i.target=-20; c.fall_rate=0;
    CHECK("UT-08", FbSetpointRamp_Execute(&c,&i,&s,&o) == FB_SETPOINT_RAMP_OK && o.value == -10);
    i.target=-10;
    CHECK("UT-09", FbSetpointRamp_Execute(&c,&i,&s,&o) == FB_SETPOINT_RAMP_OK && o.reached);
    i.enable=false; i.target=NAN; i.dt_seconds=-1; c.rise_rate=NAN;
    CHECK("UT-10", FbSetpointRamp_Execute(&c,&i,&s,&o) == FB_SETPOINT_RAMP_DISABLED && !o.valid && !o.reached && o.value == -10 && s.value == -10);
    i.enable=true; i.target=0; i.dt_seconds=1;
    CHECK("UT-11", FbSetpointRamp_Execute(&c,&i,&s,&o) == FB_SETPOINT_RAMP_INVALID_CONFIGURATION && !o.valid && s.value == -10);
    c.rise_rate=-1;
    CHECK("UT-12", FbSetpointRamp_Execute(&c,&i,&s,&o) == FB_SETPOINT_RAMP_INVALID_CONFIGURATION && s.value == -10);
    c.rise_rate=INFINITY;
    CHECK("UT-13", FbSetpointRamp_Execute(&c,&i,&s,&o) == FB_SETPOINT_RAMP_INVALID_CONFIGURATION);
    c.rise_rate=2; c.fall_rate=-1;
    CHECK("UT-14", FbSetpointRamp_Execute(&c,&i,&s,&o) == FB_SETPOINT_RAMP_INVALID_CONFIGURATION);
    c.fall_rate=NAN;
    CHECK("UT-15", FbSetpointRamp_Execute(&c,&i,&s,&o) == FB_SETPOINT_RAMP_INVALID_CONFIGURATION);
    c.fall_rate=INFINITY;
    CHECK("UT-16", FbSetpointRamp_Execute(&c,&i,&s,&o) == FB_SETPOINT_RAMP_INVALID_CONFIGURATION);
    c.fall_rate=4; saved=s.value;
    { float bad[]={NAN,INFINITY,-INFINITY}; size_t n;
      for(n=0;n<3;++n) { i.target=bad[n]; assert(FbSetpointRamp_Execute(&c,&i,&s,&o)==FB_SETPOINT_RAMP_INVALID_INPUT && !o.valid && s.value==saved); }
    }
    CHECK("UT-17", s.value==saved);
    i.target=10;
    { float bad[]={-1,NAN,INFINITY,-INFINITY}; size_t n;
      for(n=0;n<4;++n) { i.dt_seconds=bad[n]; assert(FbSetpointRamp_Execute(&c,&i,&s,&o)==FB_SETPOINT_RAMP_INVALID_INPUT && !o.valid && s.value==saved); }
    }
    CHECK("UT-18", s.value==saved);
    i.dt_seconds=0.5F;
    CHECK("UT-19", FbSetpointRamp_Execute(&c,&i,&s,&o)==FB_SETPOINT_RAMP_OK && o.value==-9 && o.valid);
    CHECK("UT-20", FbSetpointRamp_Reset(&s,3)==FB_SETPOINT_RAMP_OK && s.value==3);
    CHECK("UT-21", FbSetpointRamp_Reset(&s,NAN)==FB_SETPOINT_RAMP_INVALID_INPUT && s.value==3);
    CHECK("UT-22", FbSetpointRamp_Init(&s,INFINITY)==FB_SETPOINT_RAMP_INVALID_INPUT && s.value==3);
    CHECK("UT-23", FbSetpointRamp_Init(NULL,0)==FB_SETPOINT_RAMP_INVALID_ARGUMENT && FbSetpointRamp_Reset(NULL,0)==FB_SETPOINT_RAMP_INVALID_ARGUMENT);
    CHECK("UT-24", FbSetpointRamp_Execute(NULL,&i,&s,&o)==FB_SETPOINT_RAMP_INVALID_ARGUMENT && !o.valid && s.value==3);
    CHECK("UT-25", FbSetpointRamp_Execute(&c,NULL,&s,&o)==FB_SETPOINT_RAMP_INVALID_ARGUMENT && !o.valid && s.value==3);
    CHECK("UT-26", FbSetpointRamp_Execute(&c,&i,NULL,&o)==FB_SETPOINT_RAMP_INVALID_ARGUMENT && !o.valid && o.value==0);
    CHECK("UT-27", FbSetpointRamp_Execute(&c,&i,&s,NULL)==FB_SETPOINT_RAMP_INVALID_ARGUMENT && s.value==3);
    CHECK("UT-28", FbSetpointRamp_Execute(&c,&i,&other,&o)==FB_SETPOINT_RAMP_INVALID_STATE && !o.valid);
    s.value=NAN;
    CHECK("UT-29", FbSetpointRamp_Execute(&c,&i,&s,&o)==FB_SETPOINT_RAMP_INVALID_STATE && !o.valid && o.value==0 && isnan(s.value));
    FbSetpointRamp_Reset(&s,-FLT_MAX); c.rise_rate=FLT_MAX; i.target=FLT_MAX; i.dt_seconds=1;
    CHECK("UT-30", FbSetpointRamp_Execute(&c,&i,&s,&o)==FB_SETPOINT_RAMP_OK && o.value==0 && !o.reached);
    i.dt_seconds=FLT_MAX;
    CHECK("UT-31", FbSetpointRamp_Execute(&c,&i,&s,&o)==FB_SETPOINT_RAMP_OK && o.value==FLT_MAX && o.reached);
    c.fall_rate=FLT_MAX; i.target=-FLT_MAX; i.dt_seconds=2;
    CHECK("UT-32", FbSetpointRamp_Execute(&c,&i,&s,&o)==FB_SETPOINT_RAMP_OK && o.value==-FLT_MAX && o.reached);
    FbSetpointRamp_Reset(&s,0); FbSetpointRamp_Init(&other,20); c.rise_rate=2; i.target=10; i.dt_seconds=1;
    CHECK("UT-33", FbSetpointRamp_Execute(&c,&i,&s,&o)==FB_SETPOINT_RAMP_OK && s.value==2 && other.value==20);
    i.target=0; c.fall_rate=1;
    CHECK("UT-34", FbSetpointRamp_Execute(&c,&i,&other,&o)==FB_SETPOINT_RAMP_OK && other.value==19 && s.value==2);
    i.target=10; i.dt_seconds=-0.0F;
    CHECK("UT-35", FbSetpointRamp_Execute(&c,&i,&s,&o)==FB_SETPOINT_RAMP_OK && s.value==2);
    FbSetpointRamp_Reset(&s,32000); c.rise_rate=0.001F; c.fall_rate=0.002F;
    i.target=32001; i.dt_seconds=0.01F;
    { unsigned n; for(n=0;n<100000;++n) assert(FbSetpointRamp_Execute(&c,&i,&s,&o)==FB_SETPOINT_RAMP_OK); }
    CHECK("UT-36", s.value > 32000.99F && s.value <= 32001);
    i.target=31999;
    { unsigned n; for(n=0;n<50000;++n) assert(FbSetpointRamp_Execute(&c,&i,&s,&o)==FB_SETPOINT_RAMP_OK); }
    CHECK("UT-37", s.value >= 31999.99F && s.value <= 32000.01F);
    CHECK("UT-38", FbSetpointRamp_Reset(&s,7)==FB_SETPOINT_RAMP_OK && s.value==7 && s.precise_value==7);
    s.precise_value=8;
    CHECK("UT-39", FbSetpointRamp_Execute(&c,&i,&s,&o)==FB_SETPOINT_RAMP_INVALID_STATE && !o.valid && s.value==7 && s.precise_value==8);
    FbSetpointRamp_Reset(&s,0); i.target=1; i.dt_seconds=1; c.rise_rate=1;
    assert(FbSetpointRamp_Execute(&c,&i,&s,&o)==FB_SETPOINT_RAMP_OK);
    FbSetpointRamp_Reset(&s,32000); c.rise_rate=0.001F; i.target=32001; i.dt_seconds=0.01F;
    FbSetpointRamp_Execute(&c,&i,&s,&o);
    { double precise=s.precise_value;
      i.enable=false;
      CHECK("UT-40", FbSetpointRamp_Execute(&c,&i,&s,&o)==FB_SETPOINT_RAMP_DISABLED && s.precise_value==precise);
      i.enable=true; i.dt_seconds=0;
      CHECK("UT-41", FbSetpointRamp_Execute(&c,&i,&s,&o)==FB_SETPOINT_RAMP_OK && s.precise_value==precise);
      i.dt_seconds=1; c.rise_rate=0;
      CHECK("UT-42", FbSetpointRamp_Execute(&c,&i,&s,&o)==FB_SETPOINT_RAMP_OK && s.precise_value==precise);
    }
    s.precise_value=(double)FLT_MAX*2;
    CHECK("UT-43", FbSetpointRamp_Execute(&c,&i,&s,&o)==FB_SETPOINT_RAMP_INVALID_STATE && !o.valid);
    s.precise_value=NAN;
    CHECK("UT-44", FbSetpointRamp_Execute(&c,&i,&s,&o)==FB_SETPOINT_RAMP_INVALID_STATE && !o.valid);
    FbSetpointRamp_Reset(&s,0); c.rise_rate=1;
    assert(FbSetpointRamp_Execute(&c,&i,&s,&o)==FB_SETPOINT_RAMP_OK);
    assert(o.status==FB_SETPOINT_RAMP_OK);
    printf("FbSetpointRamp: %u cases passed; sizeof Config=%zu Input=%zu State=%zu Output=%zu\n",cases,sizeof c,sizeof i,sizeof s,sizeof o);
    return 0;
}
