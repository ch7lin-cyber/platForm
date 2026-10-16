#include "FbDeadTime.h"
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static void ok(FbDeadTimeConfig *c, FbDeadTimeState *s, float x, float y, bool ready)
{
    FbDeadTimeInput i = {x, true};
    FbDeadTimeOutput o;
    assert(FbDeadTime_Execute(c, &i, s, &o) == FB_DEAD_TIME_OK);
    assert(o.status == FB_DEAD_TIME_OK && o.valid && o.ready == ready);
    assert(memcmp(&o.value, &y, sizeof y) == 0);
}
static void held(const FbDeadTimeConfig *c, const FbDeadTimeInput *i,
    FbDeadTimeState *s, FbDeadTimeStatus expected, float *b, size_t n)
{
    unsigned char before[sizeof *s];
    float backup[8];
    FbDeadTimeOutput o;
    assert(n <= 8);
    memcpy(before, s, sizeof *s);
    if (n) memcpy(backup, b, n * sizeof(float));
    assert(FbDeadTime_Execute(c, i, s, &o) == expected);
    assert(o.status == expected && !o.valid && !o.ready && isfinite(o.value));
    assert(memcmp(before, s, sizeof *s) == 0);
    if (n) assert(memcmp(backup, b, n * sizeof(float)) == 0);
}
static void reset_bad(FbDeadTimeConfig *c, FbDeadTimeState *s, float *buf,
    size_t cap, float init, FbDeadTimeStatus expected, float *guard)
{
    unsigned char before[sizeof *s];
    float backup[5];
    memcpy(before, s, sizeof *s);
    memcpy(backup, guard, sizeof backup);
    assert(FbDeadTime_Init(c,s,buf,cap,init) == expected);
    assert(memcmp(before,s,sizeof *s) == 0 && memcmp(backup,guard,sizeof backup) == 0);
    assert(FbDeadTime_Reset(c,s,buf,cap,init) == expected);
    assert(memcmp(before,s,sizeof *s) == 0 && memcmp(backup,guard,sizeof backup) == 0);
}
int main(void)
{
    FbDeadTimeConfig c = {3}, z = {0}, one = {1};
    FbDeadTimeState s = {0}, other = {0}, saved;
    FbDeadTimeInput i = {1, true};
    FbDeadTimeOutput o;
    float b[5] = {123, NAN, NAN, NAN, 456}, copy[5], ob[1];
    const float bad[] = {NAN, INFINITY, -INFINITY};
    size_t k;
    /* UT-01: zero state rejected; Init O(1), no write to even NaN storage. */
    held(&c,&i,&s,FB_DEAD_TIME_INVALID_STATE,b,5);
    memcpy(copy,b,sizeof b);
    assert(FbDeadTime_Init(&c,&s,b+1,3,-7)==FB_DEAD_TIME_OK);
    assert(memcmp(copy,b,sizeof b)==0 && s.filled==0 && s.write_index==0);
    /* UT-02: exact N=3 boundary; first N samples initial, N+1 first input. */
    ok(&c,&s,10,-7,false); ok(&c,&s,20,-7,false); ok(&c,&s,30,-7,false);
    ok(&c,&s,40,10,true);
    /* UT-03: multiple wrap cycles, exact FIFO and guards. */
    for(k=5;k<=100;++k) ok(&c,&s,(float)k*10,(float)(k-3)*10,true);
    assert(b[0]==123 && b[4]==456 && s.filled==3);
    /* UT-04: N=0 no buffer, direct output on first valid sample. */
    assert(FbDeadTime_Init(&z,&other,NULL,0,9)==FB_DEAD_TIME_OK);
    ok(&z,&other,2,2,true); ok(&z,&other,-3,-3,true);
    /* UT-05: N=1 startup and each subsequent previous sample. */
    assert(FbDeadTime_Reset(&one,&other,ob,1,5)==FB_DEAD_TIME_OK);
    ok(&one,&other,6,5,false); ok(&one,&other,7,6,true); ok(&one,&other,8,7,true);
    /* UT-06: NO_SAMPLE holds warmed state/buffer, ignores NaN/Inf and diagnostic. */
    for(k=0;k<3;++k) { i.value=bad[k]; i.sample_valid=false;
        held(&c,&i,&s,FB_DEAD_TIME_NO_SAMPLE,b,5); }
    assert(FbDeadTime_Execute(&c,&i,&s,&o)==FB_DEAD_TIME_NO_SAMPLE && o.value==970);
    i.sample_valid=true; i.value=1010; ok(&c,&s,1010,980,true);
    /* UT-07: startup missing sample does not advance the N-sample interval. */
    assert(FbDeadTime_Reset(&c,&s,b+1,3,11)==FB_DEAD_TIME_OK);
    ok(&c,&s,1,11,false); i.sample_valid=false;
    held(&c,&i,&s,FB_DEAD_TIME_NO_SAMPLE,b,5);
    ok(&c,&s,2,11,false); ok(&c,&s,3,11,false); ok(&c,&s,4,1,true);
    /* UT-08: accepted nonfinite samples rejected atomically. */
    i.sample_valid=true;
    for(k=0;k<3;++k) { i.value=bad[k]; held(&c,&i,&s,FB_DEAD_TIME_INVALID_INPUT,b,5); }
    i.value=5; ok(&c,&s,5,2,true);
    /* UT-09: config change rejected, including NO_SAMPLE; restoration resumes. */
    c.delay_samples=1; held(&c,&i,&s,FB_DEAD_TIME_INVALID_CONFIGURATION,b,5);
    i.sample_valid=false; held(&c,&i,&s,FB_DEAD_TIME_INVALID_CONFIGURATION,b,5);
    c.delay_samples=3; i.sample_valid=true; ok(&c,&s,6,3,true);
    /* UT-10: Reset changes N/buffer/initial, leaves old/new buffers untouched. */
    memcpy(copy,b,sizeof b); ob[0]=99;
    assert(FbDeadTime_Reset(&one,&s,ob,1,-9)==FB_DEAD_TIME_OK);
    assert(memcmp(copy,b,sizeof b)==0 && ob[0]==99);
    ok(&one,&s,20,-9,false); ok(&one,&s,21,20,true);
    /* UT-11: Reset same storage discards old epoch without clearing. */
    assert(FbDeadTime_Reset(&c,&s,b+1,3,42)==FB_DEAD_TIME_OK);
    assert(memcmp(copy,b,sizeof b)==0);
    ok(&c,&s,7,42,false); ok(&c,&s,8,42,false); ok(&c,&s,9,42,false); ok(&c,&s,10,7,true);
    /* UT-12: bad capacity, impossible byte extent, NULL buffer atomic for both APIs. */
    reset_bad(&c,&s,b+1,2,0,FB_DEAD_TIME_INVALID_CONFIGURATION,b);
    reset_bad(&c,&s,NULL,3,0,FB_DEAD_TIME_INVALID_CONFIGURATION,b);
    reset_bad(&z,&s,NULL,1,0,FB_DEAD_TIME_INVALID_CONFIGURATION,b);
    reset_bad(&c,&s,b+1,(size_t)-1,0,FB_DEAD_TIME_INVALID_CONFIGURATION,b);
    { FbDeadTimeConfig huge = {(size_t)-1};
      reset_bad(&huge,&s,b+1,3,0,FB_DEAD_TIME_INVALID_CONFIGURATION,b); }
    /* UT-13: nonfinite initial and NULL config/state atomic Init and Reset. */
    for(k=0;k<3;++k) reset_bad(&c,&s,b+1,3,bad[k],FB_DEAD_TIME_INVALID_INPUT,b);
    reset_bad(NULL,&s,b+1,3,0,FB_DEAD_TIME_INVALID_ARGUMENT,b);
    assert(FbDeadTime_Init(&c,NULL,b+1,3,0)==FB_DEAD_TIME_INVALID_ARGUMENT);
    assert(FbDeadTime_Reset(&c,NULL,b+1,3,0)==FB_DEAD_TIME_INVALID_ARGUMENT);
    /* UT-14: Execute null pointers, including output, do not mutate. */
    held(NULL,&i,&s,FB_DEAD_TIME_INVALID_ARGUMENT,b,5);
    held(&c,NULL,&s,FB_DEAD_TIME_INVALID_ARGUMENT,b,5);
    assert(FbDeadTime_Execute(&c,&i,NULL,&o)==FB_DEAD_TIME_INVALID_ARGUMENT && o.value==0 && !o.valid && !o.ready);
    saved=s; memcpy(copy,b,sizeof b);
    assert(FbDeadTime_Execute(&c,&i,&s,NULL)==FB_DEAD_TIME_INVALID_ARGUMENT);
    assert(memcmp(&s,&saved,sizeof s)==0 && memcmp(copy,b,sizeof b)==0);
    /* UT-15: structural corruptions detected without touching invalid storage. */
    saved=s;
#define CORRUPT(field,val) do { s=saved; s.field=(val); held(&c,&i,&s,FB_DEAD_TIME_INVALID_STATE,b,5); } while(0)
    CORRUPT(initialized,false); CORRUPT(capacity,2); CORRUPT(capacity,(size_t)-1);
    CORRUPT(buffer,NULL); CORRUPT(write_index,3); CORRUPT(filled,4);
    CORRUPT(delay_samples,(size_t)-1);
    for(k=0;k<3;++k) { CORRUPT(initial_value,bad[k]); CORRUPT(last_value,bad[k]); }
    s=saved; s.filled=1; s.write_index=0; held(&c,&i,&s,FB_DEAD_TIME_INVALID_STATE,b,5);
    s=saved; s.filled=1; s.write_index=1; held(&c,&i,&s,FB_DEAD_TIME_INVALID_STATE,b,5);
    /* UT-16: corrupt delayed read fails before overwrite, including each nonfinite. */
    s=saved;
    for(k=0;k<3;++k) { s.buffer[s.write_index]=bad[k];
        held(&c,&i,&s,FB_DEAD_TIME_INVALID_STATE,b,5); }
    /* UT-17: no-sample ignores corrupt ring; explicit Reset repairs epoch. */
    i.sample_valid=false; held(&c,&i,&s,FB_DEAD_TIME_NO_SAMPLE,b,5); i.sample_valid=true;
    assert(FbDeadTime_Reset(&c,&s,b+1,3,-1)==FB_DEAD_TIME_OK);
    ok(&c,&s,1,-1,false); ok(&c,&s,2,-1,false); ok(&c,&s,3,-1,false); ok(&c,&s,4,1,true);
    /* UT-18: extremes, subnormals, signed zeros copied bit-for-bit, no arithmetic. */
    { const float values[] = {FLT_MAX,-FLT_MAX,FLT_MIN,-FLT_MIN,
          0.0f,-0.0f,nextafterf(0.0f,1.0f),nextafterf(0.0f,-1.0f)};
      assert(FbDeadTime_Reset(&one,&other,ob,1,-0.0f)==FB_DEAD_TIME_OK);
      for(k=0;k<sizeof values/sizeof values[0];++k)
          ok(&one,&other,values[k],k==0?-0.0f:values[k-1],k!=0);
      ok(&one,&other,1,values[k-1],true);
      assert(FbDeadTime_Reset(&z,&other,NULL,0,0)==FB_DEAD_TIME_OK);
      for(k=0;k<sizeof values/sizeof values[0];++k) ok(&z,&other,values[k],values[k],true); }
    /* UT-19: independent instances and resets never alter other state/storage. */
    saved=s; memcpy(copy,b,sizeof b);
    assert(FbDeadTime_Reset(&one,&other,ob,1,3)==FB_DEAD_TIME_OK);
    ok(&one,&other,9,3,false); ok(&one,&other,8,9,true);
    assert(memcmp(&s,&saved,sizeof s)==0 && memcmp(copy,b,sizeof b)==0);
    /* UT-20: N=0 non-NULL spare buffer untouched, no sample held; state index checks. */
    assert(FbDeadTime_Reset(&z,&other,ob,1,6)==FB_DEAD_TIME_OK); ob[0]=99;
    ok(&z,&other,7,7,true); assert(ob[0]==99);
    i.sample_valid=false; held(&z,&i,&other,FB_DEAD_TIME_NO_SAMPLE,ob,1);
    other.write_index=1; held(&z,&i,&other,FB_DEAD_TIME_INVALID_STATE,ob,1);
    assert(FbDeadTime_Reset(&z,&other,NULL,0,6)==FB_DEAD_TIME_OK);
    other.filled=1; held(&z,&i,&other,FB_DEAD_TIME_INVALID_STATE,ob,1);
    /* UT-21: surplus capacity and sentinels remain untouched. */
    assert(FbDeadTime_Reset(&one,&s,b+1,3,1)==FB_DEAD_TIME_OK);
    b[2]=88; b[3]=89;
    for(k=0;k<100;++k) ok(&one,&s,(float)k,k==0?1:(float)(k-1),k!=0);
    assert(b[0]==123 && b[2]==88 && b[3]==89 && b[4]==456);
    printf("FbDeadTime: UT-01..UT-21 PASS; sizeof Config=%zu Input=%zu State=%zu Output=%zu\n",
        sizeof c,sizeof i,sizeof s,sizeof o);
    return 0;
}
