#include "FbPidGroupSelector.h"
#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
int main(void)
{
    FbPidGroupSelectorConfig c, original;
    FbPidGroupSelectorInput in={0,true,true,false,0};
    FbPidGroupSelectorOutput out;
#define RUN() FbPidGroupSelector_Execute(&c,&in,&out)
    FbPidGroupSelector_DefaultConfig(&c);
    assert(FbPidGroupSelector_ValidateConfig(&c));
    for (unsigned g=0;g<4;++g) {
        c.groups[g].pb=1000+g;
        c.groups[g].cool_pb=2000+g;
        c.groups[g].ti_ms=3000+g;
        c.groups[g].td_ms=4000+g;
        c.groups[g].cool_ti_ms=5000+g;
        c.groups[g].cool_td_ms=6000+g;
        c.groups[g].integral_preload=(uint16_t)(700+g);
        c.groups[g].alpha=(uint16_t)(10+g);
        c.groups[g].beta=(uint16_t)(20+g);
        c.groups[g].gamma=(uint16_t)(30+g);
        c.selection=(uint8_t)g; in.sv_valid=false;
        assert(RUN()==FB_PID_GROUP_OK && out.group==g && out.valid);
        assert(out.parameters.pb==1000+g && out.parameters.cool_pb==2000+g);
        assert(out.parameters.ti_ms==3000+g && out.parameters.td_ms==4000+g);
        assert(out.parameters.cool_ti_ms==5000+g && out.parameters.cool_td_ms==6000+g);
        assert(out.parameters.integral_preload==700+g && out.parameters.alpha==10+g);
        assert(out.parameters.beta==20+g && out.parameters.gamma==30+g);
    }
    c.selection=FB_PID_GROUP_AUTO;
    c.switch_sv[0]=-1000; c.switch_sv[1]=0; c.switch_sv[2]=1000;
    in.sv_valid=true;
    { const int32_t sv[]={INT_MIN,-1001,-1000,-1,0,999,1000,INT_MAX};
      const unsigned expected[]={0,0,1,1,2,2,3,3};
      for (unsigned i=0;i<8;++i) {
        in.sv=sv[i]; assert(RUN()==FB_PID_GROUP_OK && out.group==expected[i]);
      }
    }
    c.switch_sv[0]=c.switch_sv[1]=c.switch_sv[2]=0;
    in.sv=0; RUN(); assert(out.group==3);
    in.sv=-1; RUN(); assert(out.group==0);
    in.sv_valid=false;
    assert(RUN()==FB_PID_GROUP_INVALID_INPUT && !out.valid && out.parameters.pb==0);
    in.lock_group=true; in.locked_group=2;
    assert(RUN()==FB_PID_GROUP_OK && out.group==2 && out.locked);
    in.locked_group=4;
    assert(RUN()==FB_PID_GROUP_INVALID_INPUT);
    in.locked_group=0; original=c;
    /* Validate all groups, including unselected cooling coefficients. */
    c.groups[3].cool_pb=0;
    assert(RUN()==FB_PID_GROUP_INVALID_CONFIG);
    c=original; c.switch_sv[1]=-1;
    assert(RUN()==FB_PID_GROUP_INVALID_CONFIG);
    c=original; c.selection=5;
    assert(RUN()==FB_PID_GROUP_INVALID_CONFIG);
    c=original; c.groups[0].ti_ms=30000001;
    assert(RUN()==FB_PID_GROUP_INVALID_CONFIG);
    c=original; c.groups[1].gamma=201;
    assert(RUN()==FB_PID_GROUP_INVALID_CONFIG);
    c=original; c.groups[2].integral_preload=1001;
    assert(RUN()==FB_PID_GROUP_INVALID_CONFIG);
    c=original; c.groups[0].pb=3000001;
    assert(RUN()==FB_PID_GROUP_INVALID_CONFIG);
    c=original; c.groups[0]=(FbPidGroupParameters){1,3000000,0,30000000,0,30000000,1000,0,200,0};
    assert(RUN()==FB_PID_GROUP_OK);
    /* Returned packet is a value copy, and calls have no retained state. */
    c.groups[0].pb=2; assert(out.parameters.pb==1);
    assert(RUN()==FB_PID_GROUP_OK && out.parameters.pb==2);
    in.enable=false; c.selection=99;
    assert(RUN()==FB_PID_GROUP_DISABLED && !out.valid && out.parameters.pb==0);
    assert(FbPidGroupSelector_Execute(NULL,&in,&out)==FB_PID_GROUP_INVALID_ARGUMENT);
    assert(FbPidGroupSelector_Execute(&c,NULL,&out)==FB_PID_GROUP_INVALID_ARGUMENT);
    assert(FbPidGroupSelector_Execute(&c,&in,NULL)==FB_PID_GROUP_INVALID_ARGUMENT);
    assert(!FbPidGroupSelector_ValidateConfig(NULL));
    FbPidGroupSelector_DefaultConfig(NULL);
    puts("FbPidGroupSelector: fixed/auto, boundary, lock, complete bank validation and packet copy PASS");
    return 0;
}
