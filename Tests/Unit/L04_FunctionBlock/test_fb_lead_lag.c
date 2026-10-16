#include "FbLeadLag.h"
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
static void ok(const FbLeadLagConfig *c, const FbLeadLagInput *i,
               FbLeadLagState *s, FbLeadLagOutput *o, float expected)
{
    assert(FbLeadLag_Execute(c,i,s,o)==FB_LEAD_LAG_OK);
    assert(o->valid && o->status==FB_LEAD_LAG_OK && o->value==expected);
}
static void held(const FbLeadLagConfig *c, const FbLeadLagInput *i,
                 FbLeadLagState *s, FbLeadLagOutput *o, FbLeadLagStatus status)
{
    unsigned char snapshot[sizeof(*s)];
    memcpy(snapshot,s,sizeof(*s));
    assert(FbLeadLag_Execute(c,i,s,o)==status);
    assert(!o->valid && o->status==status && isfinite(o->value));
    assert(memcmp(snapshot,s,sizeof(*s))==0);
}
int main(void)
{
    FbLeadLagConfig c={0,1};
    FbLeadLagInput i={8,1,true};
    FbLeadLagState s={0}, other={0}, before;
    FbLeadLagOutput o;
    const float bad[]={NAN,INFINITY,-INFINITY};
    size_t k;
    /* UT-01: zero state rejected; explicit equilibrium startup. */
    held(&c,&i,&s,&o,FB_LEAD_LAG_INVALID_STATE);
    assert(FbLeadLag_Init(&c,&s,8)==FB_LEAD_LAG_OK);
    ok(&c,&i,&s,&o,8);
    /* UT-02: lowpass step has analytic BE values, no artificial kick. */
    assert(FbLeadLag_Reset(&c,&s,0)==FB_LEAD_LAG_OK);
    ok(&c,&i,&s,&o,4); ok(&c,&i,&s,&o,6); ok(&c,&i,&s,&o,7);
    /* UT-03: variable dt uses current accepted interval. */
    i.dt_seconds=3; ok(&c,&i,&s,&o,7.75f);
    /* UT-04: real lead step overshoot then convergence, constant reset. */
    c.lead_seconds=3; i.dt_seconds=1;
    assert(FbLeadLag_Reset(&c,&s,0)==FB_LEAD_LAG_OK);
    ok(&c,&i,&s,&o,16); ok(&c,&i,&s,&o,12); ok(&c,&i,&s,&o,10);
    assert(FbLeadLag_Reset(&c,&s,8)==FB_LEAD_LAG_OK); ok(&c,&i,&s,&o,8);
    /* UT-05: exact identity even huge opposite endpoints and tiny dt. */
    c.lead_seconds=c.lag_seconds=FLT_MAX;
    assert(FbLeadLag_Reset(&c,&s,-FLT_MAX)==FB_LEAD_LAG_OK);
    i.value=FLT_MAX; i.dt_seconds=nextafterf(0,1); ok(&c,&i,&s,&o,FLT_MAX);
    i.value=1; ok(&c,&i,&s,&o,1);
    /* UT-06: NO_SAMPLE ignores NaN numerics; no mutation/catch-up. */
    i.sample_valid=false; i.value=NAN; i.dt_seconds=NAN;
    held(&c,&i,&s,&o,FB_LEAD_LAG_NO_SAMPLE);
    i.sample_valid=true; i.value=8; i.dt_seconds=1;
    /* UT-07: epoch config changed rejected, reset rebinds including same. */
    c.lead_seconds=0; c.lag_seconds=1;
    held(&c,&i,&s,&o,FB_LEAD_LAG_INVALID_CONFIGURATION);
    assert(FbLeadLag_Reset(&c,&s,0)==FB_LEAD_LAG_OK); ok(&c,&i,&s,&o,4);
    assert(FbLeadLag_Reset(&c,&s,0)==FB_LEAD_LAG_OK); ok(&c,&i,&s,&o,4);
    /* UT-08: input NaN/Inf/-Inf and zero/negative dt reject atomically. */
    for(k=0;k<3;++k) {
        i.value=bad[k]; held(&c,&i,&s,&o,FB_LEAD_LAG_INVALID_INPUT); i.value=8;
        i.dt_seconds=bad[k]; held(&c,&i,&s,&o,FB_LEAD_LAG_INVALID_INPUT); i.dt_seconds=1;
    }
    i.dt_seconds=0; held(&c,&i,&s,&o,FB_LEAD_LAG_INVALID_INPUT);
    i.dt_seconds=-1; held(&c,&i,&s,&o,FB_LEAD_LAG_INVALID_INPUT); i.dt_seconds=1;
    /* UT-09: invalid config and failed Init/Reset leave state intact. */
    for(k=0;k<3;++k) {
        before=s; c.lead_seconds=bad[k];
        held(&c,&i,&s,&o,FB_LEAD_LAG_INVALID_CONFIGURATION);
        assert(FbLeadLag_Reset(&c,&s,0)==FB_LEAD_LAG_INVALID_CONFIGURATION);
        assert(memcmp(&s,&before,sizeof(s))==0); c.lead_seconds=0;
        c.lag_seconds=bad[k]; held(&c,&i,&s,&o,FB_LEAD_LAG_INVALID_CONFIGURATION); c.lag_seconds=1;
        assert(FbLeadLag_Init(&c,&s,bad[k])==FB_LEAD_LAG_INVALID_INPUT);
        assert(memcmp(&s,&before,sizeof(s))==0);
    }
    c.lead_seconds=-1; held(&c,&i,&s,&o,FB_LEAD_LAG_INVALID_CONFIGURATION); c.lead_seconds=0;
    c.lag_seconds=0; held(&c,&i,&s,&o,FB_LEAD_LAG_INVALID_CONFIGURATION);
    c.lag_seconds=-1; held(&c,&i,&s,&o,FB_LEAD_LAG_INVALID_CONFIGURATION); c.lag_seconds=1;
    /* UT-10: null arguments and all observable state corruption checks. */
    held(NULL,&i,&s,&o,FB_LEAD_LAG_INVALID_ARGUMENT);
    held(&c,NULL,&s,&o,FB_LEAD_LAG_INVALID_ARGUMENT);
    assert(FbLeadLag_Execute(&c,&i,NULL,&o)==FB_LEAD_LAG_INVALID_ARGUMENT);
    before=s; assert(FbLeadLag_Execute(&c,&i,&s,NULL)==FB_LEAD_LAG_INVALID_ARGUMENT);
    assert(memcmp(&s,&before,sizeof(s))==0);
    assert(FbLeadLag_Init(NULL,&s,0)==FB_LEAD_LAG_INVALID_ARGUMENT);
    assert(FbLeadLag_Reset(&c,NULL,0)==FB_LEAD_LAG_INVALID_ARGUMENT);
    s.lag_value=INFINITY; held(&c,&i,&s,&o,FB_LEAD_LAG_INVALID_STATE); s=before;
    s.lag_value=2.0*(double)FLT_MAX; held(&c,&i,&s,&o,FB_LEAD_LAG_INVALID_STATE); s=before;
    s.value=NAN; held(&c,&i,&s,&o,FB_LEAD_LAG_INVALID_STATE); s=before;
    s.active_lead_seconds=-1; held(&c,&i,&s,&o,FB_LEAD_LAG_INVALID_STATE); s=before;
    s.active_lag_seconds=0; held(&c,&i,&s,&o,FB_LEAD_LAG_INVALID_STATE); s=before;
    /* UT-11: extreme lead overflow rejects candidates, does not poison q. */
    c.lead_seconds=FLT_MAX; c.lag_seconds=nextafterf(0,1);
    assert(FbLeadLag_Reset(&c,&s,-FLT_MAX)==FB_LEAD_LAG_OK);
    i.value=FLT_MAX; i.dt_seconds=c.lag_seconds;
    held(&c,&i,&s,&o,FB_LEAD_LAG_NUMERIC_ERROR);
    i.value=-FLT_MAX; ok(&c,&i,&s,&o,-FLT_MAX);
    /* UT-12: alpha rounds to one, yet representable decay tail survives. */
    c.lead_seconds=0; c.lag_seconds=FLT_MIN;
    assert(FbLeadLag_Reset(&c,&s,FLT_MAX)==FB_LEAD_LAG_OK);
    i.value=0; i.dt_seconds=FLT_MAX; ok(&c,&i,&s,&o,FLT_MIN);
    c.lag_seconds=nextafterf(0,1);
    assert(FbLeadLag_Reset(&c,&s,FLT_MAX)==FB_LEAD_LAG_OK);
    ok(&c,&i,&s,&o,c.lag_seconds);
    /* UT-13: sub-float increments accumulate in double instead of freezing. */
    c.lag_seconds=1; assert(FbLeadLag_Reset(&c,&s,1)==FB_LEAD_LAG_OK);
    i.value=2; i.dt_seconds=1e-9f;
    for(k=0;k<1000;++k) assert(FbLeadLag_Execute(&c,&i,&s,&o)==FB_LEAD_LAG_OK);
    assert(o.value>1 && s.lag_value>1.0000009 && s.lag_value<1.0000011);
    /* UT-14: opposite extreme lowpass endpoints stay finite and bounded. */
    assert(FbLeadLag_Reset(&c,&s,-FLT_MAX)==FB_LEAD_LAG_OK);
    i.value=FLT_MAX; i.dt_seconds=1; ok(&c,&i,&s,&o,0);
    /* UT-15: independent instances never share history. */
    assert(FbLeadLag_Init(&c,&other,20)==FB_LEAD_LAG_OK);
    i.value=8; ok(&c,&i,&s,&o,4); ok(&c,&i,&other,&o,14);
    /* UT-16: deterministic variable-dt sequence against long-double oracle. */
    for(k=0;k<3;++k) {
        size_t n; long double q=0;
        c.lead_seconds=(float)k*0.75f; c.lag_seconds=0.7f;
        assert(FbLeadLag_Reset(&c,&s,0)==FB_LEAD_LAG_OK);
        for(n=0;n<10000;++n) {
            long double delta,y,denom;
            i.value=(float)((int)(n%97)-48)*0.125f;
            i.dt_seconds=(float)(n%11+1)*0.013f;
            denom=(long double)c.lag_seconds+i.dt_seconds;
            delta=(long double)i.value-q;
            q+=(long double)i.dt_seconds/denom*delta;
            y=q+(long double)c.lead_seconds/denom*delta;
            assert(FbLeadLag_Execute(&c,&i,&s,&o)==FB_LEAD_LAG_OK);
            assert(fabsl((long double)o.value-y)<=2e-6L*fmaxl(1,fabsl(y)));
        }
    }
    /* UT-17: near-identity and tiny-dt decaying lead/lag remain bounded. */
    c.lag_seconds=FLT_MIN; c.lead_seconds=nextafterf(FLT_MIN,0);
    assert(FbLeadLag_Reset(&c,&s,FLT_MAX)==FB_LEAD_LAG_OK);
    i.value=0; i.dt_seconds=FLT_MAX;
    ok(&c,&i,&s,&o,c.lag_seconds-c.lead_seconds);
    /* UT-18: independently rounded convex weights cannot overflow equilibrium. */
    c.lead_seconds=0x1.1ad306p-101f; c.lag_seconds=0x1.3c547ap-48f;
    assert(FbLeadLag_Reset(&c,&s,FLT_MAX)==FB_LEAD_LAG_OK);
    i.value=FLT_MAX; i.dt_seconds=0x1.cd22e2p-12f; ok(&c,&i,&s,&o,FLT_MAX);
    assert(FbLeadLag_Reset(&c,&s,-FLT_MAX)==FB_LEAD_LAG_OK);
    i.value=-FLT_MAX; ok(&c,&i,&s,&o,-FLT_MAX);
    printf("FbLeadLag: UT-01..UT-18 PASS (30000 oracle samples)\n");
    return 0;
}
