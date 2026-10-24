#include "FbSignalLinearization.h"
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>

int main(void)
{
    const FbLinearizationPoint points[] = {{-10, 10}, {0, 20}, {10, -10}};
    FbSignalLinearizationParameters p = {0};
    FbInputLinearizationConfig ic = {0};
    FbOutputLinearizationConfig oc = {0};
    FbSignalLinearizationInput in = {-5, true, true};
    FbSignalLinearizationOutput out;
    p.curve = (FbLinearizationConfig){points, 3, FB_LINEARIZATION_CLAMP};
    ic.mode = FB_INPUT_LINEARIZATION_VALUE;
    ic.parameters = p;
    oc.mode = FB_OUTPUT_LINEARIZATION_INTERPOLATED;
    oc.parameters = p;
    assert(FbInputLinearization_Execute(&ic, &in, &out) == FB_LINEARIZATION_OK);
    assert(out.valid && out.value == 15);
    ic.mode = FB_INPUT_LINEARIZATION_DEVIATION;
    ic.parameters.offset_enabled = true;
    ic.parameters.offset = 2;
    assert(FbInputLinearization_Execute(&ic, &in, &out) == FB_LINEARIZATION_OK);
    assert(out.value == 12);
    ic.mode = FB_INPUT_LINEARIZATION_BYPASS;
    ic.parameters.curve.table = NULL;
    assert(FbInputLinearization_Execute(&ic, &in, &out) == FB_LINEARIZATION_OK);
    assert(out.bypassed && out.value == -5);
    oc.mode = FB_OUTPUT_LINEARIZATION_STEPPED;
    { const float x[] = {-11,-10,-5,0,5,10,11};
      const float y[] = {10,10,10,20,20,-10,-10};
      size_t i;
      for (i=0;i<7;++i) {
        in.value=x[i];
        assert(FbOutputLinearization_Execute(&oc,&in,&out)==FB_LINEARIZATION_OK);
        assert(out.value==y[i]);
      }
    }
    in.value = -11;
    oc.parameters.curve.policy = FB_LINEARIZATION_ERROR;
    assert(FbOutputLinearization_Execute(&oc,&in,&out)==FB_LINEARIZATION_OUT_OF_RANGE);
    assert(!out.valid && out.value==0 && !out.limited);
    oc.parameters.curve.policy = FB_LINEARIZATION_CLAMP;
    oc.parameters.offset_enabled = true;
    oc.parameters.offset = 5;
    oc.parameters.limits_enabled = true;
    oc.parameters.limit_low = 0;
    oc.parameters.limit_high = 12;
    oc.parameters.limit_policy = FB_LINEARIZATION_CLAMP;
    assert(FbOutputLinearization_Execute(&oc,&in,&out)==FB_LINEARIZATION_OK);
    assert(out.value==12 && out.limited && out.range==FB_LINEARIZATION_RANGE_BELOW);
    oc.parameters.limit_policy = FB_LINEARIZATION_ERROR;
    assert(FbOutputLinearization_Execute(&oc,&in,&out)==FB_LINEARIZATION_OUT_OF_RANGE);
    oc.parameters.limit_low=20;
    assert(FbOutputLinearization_Execute(&oc,&in,&out)==FB_LINEARIZATION_INVALID_CONFIGURATION);
    oc.parameters=p;
    { const FbLinearizationPoint bad[]={{0,0},{0,1}};
      oc.parameters.curve.table=bad; oc.parameters.curve.count=2;
      assert(FbOutputLinearization_Execute(&oc,&in,&out)==FB_LINEARIZATION_INVALID_CONFIGURATION);
    }
    oc.parameters=p;
    in.valid=false;
    assert(FbOutputLinearization_Execute(&oc,&in,&out)==FB_LINEARIZATION_INVALID_INPUT);
    in.valid=true; in.value=NAN;
    assert(FbOutputLinearization_Execute(&oc,&in,&out)==FB_LINEARIZATION_INVALID_INPUT);
    oc.mode=FB_OUTPUT_LINEARIZATION_OFF;
    assert(FbOutputLinearization_Execute(&oc,&in,&out)==FB_LINEARIZATION_DISABLED);
    in.enable=false; ic.mode=(FbInputLinearizationMode)99;
    assert(FbInputLinearization_Execute(&ic,&in,&out)==FB_LINEARIZATION_DISABLED);
    assert(FbInputLinearization_Execute(NULL,&in,&out)==FB_LINEARIZATION_INVALID_ARGUMENT);
    assert(FbOutputLinearization_Execute(&oc,&in,NULL)==FB_LINEARIZATION_INVALID_ARGUMENT);
    in.enable=true; in.value=0;
    assert(FbInputLinearization_Execute(&ic,&in,&out)==FB_LINEARIZATION_INVALID_CONFIGURATION);
    { const FbLinearizationPoint big[]={{-1,FLT_MAX},{1,FLT_MAX}};
      oc.mode=FB_OUTPUT_LINEARIZATION_INTERPOLATED; oc.parameters=p;
      oc.parameters.curve.table=big; oc.parameters.curve.count=2;
      oc.parameters.offset_enabled=true; oc.parameters.offset=FLT_MAX;
      assert(FbOutputLinearization_Execute(&oc,&in,&out)==FB_LINEARIZATION_INVALID_INPUT);
      oc.parameters.limits_enabled=true; oc.parameters.limit_low=0;
      oc.parameters.limit_high=100; oc.parameters.limit_policy=FB_LINEARIZATION_CLAMP;
      assert(FbOutputLinearization_Execute(&oc,&in,&out)==FB_LINEARIZATION_OK);
      assert(out.value==100 && out.limited);
    }
    puts("signal linearization modes, boundaries, quality, limits and overflow: PASS");
    return 0;
}
