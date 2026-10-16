#include "FbOutputLimiter.h"

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>

static void check(FbOutputLimiterConfig *c, FbOutputLimiterInput *i,
                  FbOutputLimiterState *s, FbOutputLimiterOutput *o,
                  float expected, int amplitude, int rate)
{
    assert(FbOutputLimiter_Execute(c, i, s, o) == FB_OUTPUT_LIMITER_OK);
    assert(o->status == FB_OUTPUT_LIMITER_OK && o->valid);
    assert(o->value == expected && s->value == expected);
    assert(o->amplitude_limited == amplitude && o->rate_limited == rate);
    assert(o->value >= c->lower_limit && o->value <= c->upper_limit);
}

static void invalid(FbOutputLimiterConfig *c, FbOutputLimiterInput *i,
                    FbOutputLimiterState *s, FbOutputLimiterOutput *o,
                    FbOutputLimiterStatus expected)
{
    float held = s->value;
    assert(FbOutputLimiter_Execute(c, i, s, o) == expected);
    assert(o->status == expected && !o->valid);
    assert(!o->amplitude_limited && !o->rate_limited);
    assert(s->value == held && o->value == held);
}

int main(void)
{
    FbOutputLimiterConfig c = {-10.0f, 10.0f, 2.0f, 4.0f};
    FbOutputLimiterInput i = {8.0f, 0.5f, 1};
    FbOutputLimiterState s = {0.0f, 0.0, 0};
    FbOutputLimiterState other = {0.0f, 0.0, 0};
    FbOutputLimiterOutput o;
    const float bad[] = {NAN, INFINITY, -INFINITY};
    size_t k;

    /* UT-01: lifecycle and explicit initialization. */
    invalid(&c, &i, &s, &o, FB_OUTPUT_LIMITER_INVALID_STATE);
    assert(FbOutputLimiter_Init(&s, 0.0f) == FB_OUTPUT_LIMITER_OK);
    assert(s.initialized == 1 && s.value == 0.0f);
    /* UT-02: rising rate and fractional seconds. */
    check(&c, &i, &s, &o, 1.0f, 0, 1);
    check(&c, &i, &s, &o, 2.0f, 0, 1);
    /* UT-03: independent falling rate. */
    i.requested = -8.0f;
    check(&c, &i, &s, &o, 0.0f, 0, 1);
    /* UT-04: exact arrival, no overshoot, stable target. */
    i.requested = 1.0f;
    check(&c, &i, &s, &o, 1.0f, 0, 0);
    check(&c, &i, &s, &o, 1.0f, 0, 0);
    i.requested = 0.5f;
    check(&c, &i, &s, &o, 0.5f, 0, 0);
    /* UT-05: upper/lower clamps, combined amplitude and slew flags. */
    i.requested = 99.0f;
    check(&c, &i, &s, &o, 1.5f, 1, 1);
    i.dt_seconds = 100.0f;
    check(&c, &i, &s, &o, 10.0f, 1, 0);
    i.requested = -99.0f;
    check(&c, &i, &s, &o, -10.0f, 1, 0);
    /* UT-06: zero dt holds, then resumes without accumulated time. */
    i.requested = 0.0f; i.dt_seconds = 0.0f;
    check(&c, &i, &s, &o, -10.0f, 0, 1);
    i.dt_seconds = 0.5f;
    check(&c, &i, &s, &o, -9.0f, 0, 1);
    /* UT-07: zero rate independently holds each direction. */
    c.rise_rate = 0.0f;
    check(&c, &i, &s, &o, -9.0f, 0, 1);
    c.rise_rate = 2.0f; c.fall_rate = 0.0f; i.requested = -10.0f;
    check(&c, &i, &s, &o, -9.0f, 0, 1);
    c.fall_rate = 4.0f;
    /* UT-08: tightened bounds override zero time and zero rates. */
    c.lower_limit = -2.0f; c.upper_limit = 2.0f;
    c.rise_rate = 0.0f; c.fall_rate = 0.0f; i.dt_seconds = 0.0f;
    check(&c, &i, &s, &o, -2.0f, 1, 0);
    assert(FbOutputLimiter_Reset(&s, 9.0f) == FB_OUTPUT_LIMITER_OK);
    i.requested = 0.0f;
    check(&c, &i, &s, &o, 2.0f, 1, 1);
    /* UT-09: equal bounds, exact-boundary request not marked clipped. */
    c.lower_limit = c.upper_limit = 1.0f; i.requested = 1.0f;
    check(&c, &i, &s, &o, 1.0f, 1, 0);
    check(&c, &i, &s, &o, 1.0f, 0, 0);
    /* UT-10: disabled ignores nonfinite request/time, enforces bounds. */
    i.enable = 0; i.requested = NAN; i.dt_seconds = -INFINITY;
    assert(FbOutputLimiter_Reset(&s, 9.0f) == FB_OUTPUT_LIMITER_OK);
    assert(FbOutputLimiter_Execute(&c, &i, &s, &o) == FB_OUTPUT_LIMITER_DISABLED);
    assert(o.status == FB_OUTPUT_LIMITER_DISABLED && !o.valid);
    assert(o.value == 1.0f && s.value == 1.0f && o.amplitude_limited && !o.rate_limited);
    assert(FbOutputLimiter_Execute(&c, &i, &s, &o) == FB_OUTPUT_LIMITER_DISABLED);
    assert(o.value == 1.0f && !o.amplitude_limited);
    /* UT-11: resume uses current dt only. */
    c.lower_limit = -10.0f; c.upper_limit = 10.0f;
    c.rise_rate = 2.0f; c.fall_rate = 4.0f;
    i.enable = 1; i.requested = 8.0f; i.dt_seconds = 0.5f;
    check(&c, &i, &s, &o, 2.0f, 0, 1);
    /* UT-12: nonfinite request/dt; negative dt; atomic errors. */
    for (k = 0; k < sizeof(bad) / sizeof(bad[0]); ++k) {
        i.requested = bad[k];
        invalid(&c, &i, &s, &o, FB_OUTPUT_LIMITER_INVALID_INPUT);
        i.requested = 8.0f; i.dt_seconds = bad[k];
        invalid(&c, &i, &s, &o, FB_OUTPUT_LIMITER_INVALID_INPUT);
        i.dt_seconds = 0.5f;
    }
    i.dt_seconds = -1.0f;
    invalid(&c, &i, &s, &o, FB_OUTPUT_LIMITER_INVALID_INPUT);
    i.dt_seconds = 0.5f;
    /* UT-13: each config field rejects NaN and both infinities. */
    for (k = 0; k < sizeof(bad) / sizeof(bad[0]); ++k) {
        c.lower_limit = bad[k]; invalid(&c, &i, &s, &o, FB_OUTPUT_LIMITER_INVALID_CONFIGURATION); c.lower_limit = -10.0f;
        c.upper_limit = bad[k]; invalid(&c, &i, &s, &o, FB_OUTPUT_LIMITER_INVALID_CONFIGURATION); c.upper_limit = 10.0f;
        c.rise_rate = bad[k]; invalid(&c, &i, &s, &o, FB_OUTPUT_LIMITER_INVALID_CONFIGURATION); c.rise_rate = 2.0f;
        c.fall_rate = bad[k]; invalid(&c, &i, &s, &o, FB_OUTPUT_LIMITER_INVALID_CONFIGURATION); c.fall_rate = 4.0f;
    }
    /* UT-14: inverted bounds and negative rates, including disabled. */
    c.lower_limit = 11.0f; invalid(&c, &i, &s, &o, FB_OUTPUT_LIMITER_INVALID_CONFIGURATION); c.lower_limit = -10.0f;
    c.rise_rate = -1.0f; invalid(&c, &i, &s, &o, FB_OUTPUT_LIMITER_INVALID_CONFIGURATION); c.rise_rate = 2.0f;
    c.fall_rate = -1.0f; i.enable = 0;
    invalid(&c, &i, &s, &o, FB_OUTPUT_LIMITER_INVALID_CONFIGURATION); c.fall_rate = 4.0f; i.enable = 1;
    /* UT-15: every null pointer; errors never change state. */
    invalid(NULL, &i, &s, &o, FB_OUTPUT_LIMITER_INVALID_ARGUMENT);
    invalid(&c, NULL, &s, &o, FB_OUTPUT_LIMITER_INVALID_ARGUMENT);
    assert(FbOutputLimiter_Execute(&c, &i, NULL, &o) == FB_OUTPUT_LIMITER_INVALID_ARGUMENT);
    assert(o.value == 0.0f && !o.valid && o.status == FB_OUTPUT_LIMITER_INVALID_ARGUMENT);
    assert(FbOutputLimiter_Execute(&c, &i, &s, NULL) == FB_OUTPUT_LIMITER_INVALID_ARGUMENT);
    assert(s.value == 2.0f);
    assert(FbOutputLimiter_Init(NULL, 0.0f) == FB_OUTPUT_LIMITER_INVALID_ARGUMENT);
    assert(FbOutputLimiter_Reset(NULL, 0.0f) == FB_OUTPUT_LIMITER_INVALID_ARGUMENT);
    /* UT-16: invalid initial/reset values preserve established history. */
    for (k = 0; k < sizeof(bad) / sizeof(bad[0]); ++k) {
        assert(FbOutputLimiter_Init(&s, bad[k]) == FB_OUTPUT_LIMITER_INVALID_INPUT);
        assert(FbOutputLimiter_Reset(&s, bad[k]) == FB_OUTPUT_LIMITER_INVALID_INPUT);
        assert(s.value == 2.0f && s.initialized == 1);
    }
    /* UT-17: damaged state fails closed; reset recovers. */
    s.initialized = false;
    assert(FbOutputLimiter_Execute(&c, &i, &s, &o) == FB_OUTPUT_LIMITER_INVALID_STATE);
    assert(!o.valid && o.value == 0.0f && !s.initialized);
    s.initialized = 1; s.value = NAN;
    assert(FbOutputLimiter_Execute(&c, &i, &s, &o) == FB_OUTPUT_LIMITER_INVALID_STATE);
    assert(!o.valid && o.value == 0.0f && isnan(s.value));
    assert(FbOutputLimiter_Reset(&s, 0.0f) == FB_OUTPUT_LIMITER_OK);
    check(&c, &i, &s, &o, 1.0f, 0, 1);
    /* UT-18: independent instances, reset one without affecting the other. */
    assert(FbOutputLimiter_Init(&other, -5.0f) == FB_OUTPUT_LIMITER_OK);
    check(&c, &i, &other, &o, -4.0f, 0, 1);
    assert(s.value == 1.0f);
    assert(FbOutputLimiter_Reset(&s, 3.0f) == FB_OUTPUT_LIMITER_OK);
    assert(other.value == -4.0f);
    /* UT-19: full float range, huge dt/rate, both directions. */
    c.lower_limit = -FLT_MAX; c.upper_limit = FLT_MAX;
    c.rise_rate = FLT_MAX; c.fall_rate = FLT_MAX;
    i.requested = FLT_MAX; i.dt_seconds = FLT_MAX;
    assert(FbOutputLimiter_Reset(&s, -FLT_MAX) == FB_OUTPUT_LIMITER_OK);
    check(&c, &i, &s, &o, FLT_MAX, 0, 0);
    i.requested = -FLT_MAX;
    check(&c, &i, &s, &o, -FLT_MAX, 0, 0);
    /* UT-20: partial crossing of extreme span and tiny positive step. */
    i.requested = FLT_MAX; i.dt_seconds = 1.0f;
    check(&c, &i, &s, &o, 0.0f, 0, 1);
    c.rise_rate = FLT_MIN; i.dt_seconds = 1.0f; i.requested = 1.0f;
    check(&c, &i, &s, &o, FLT_MIN, 0, 1);

    /* UT-21: accumulate sub-float-resolution steps instead of stalling. */
    c.lower_limit = -FLT_MAX; c.upper_limit = FLT_MAX;
    c.rise_rate = 0.001f; c.fall_rate = 0.001f;
    i.dt_seconds = 0.01f; i.requested = 32001.0f;
    assert(FbOutputLimiter_Reset(&s, 32000.0f) == FB_OUTPUT_LIMITER_OK);
    for (k = 0; k < 100000U; ++k) {
        assert(FbOutputLimiter_Execute(&c, &i, &s, &o) == FB_OUTPUT_LIMITER_OK);
    }
    assert(o.value == 32001.0f && s.precise_value > 32000.99);
    /* UT-22: reverse tiny steps, then reset clears fractional history. */
    i.requested = 32000.0f;
    for (k = 0; k < 100000U; ++k) {
        assert(FbOutputLimiter_Execute(&c, &i, &s, &o) == FB_OUTPUT_LIMITER_OK);
    }
    assert(o.value == 32000.0f && s.precise_value < 32000.01);
    assert(FbOutputLimiter_Reset(&s, 32000.0f) == FB_OUTPUT_LIMITER_OK);
    assert(s.precise_value == 32000.0);
    /* UT-23: bounds correct both accumulator and public value; coherent state. */
    c.upper_limit = 31999.0f; i.dt_seconds = 0.0f;
    check(&c, &i, &s, &o, 31999.0f, 1, 0);
    assert(s.precise_value == 31999.0);
    s.precise_value = 1.0;
    invalid(&c, &i, &s, &o, FB_OUTPUT_LIMITER_INVALID_STATE);
    assert(FbOutputLimiter_Reset(&s, 0.0f) == FB_OUTPUT_LIMITER_OK);
    s.precise_value = INFINITY;
    invalid(&c, &i, &s, &o, FB_OUTPUT_LIMITER_INVALID_STATE);

    /* UT-24: out-of-float-range accumulator rejected before narrowing. */
    s.precise_value = (double)FLT_MAX * 2.0;
    invalid(&c, &i, &s, &o, FB_OUTPUT_LIMITER_INVALID_STATE);
    /* UT-25: disabled, zero dt and zero rate preserve fractional history. */
    assert(FbOutputLimiter_Reset(&s, 32000.0f) == FB_OUTPUT_LIMITER_OK);
    c.upper_limit = FLT_MAX; i.requested = 32001.0f; i.dt_seconds = 0.01f;
    assert(FbOutputLimiter_Execute(&c, &i, &s, &o) == FB_OUTPUT_LIMITER_OK);
    {
        double held = s.precise_value;
        assert(held > 32000.0 && s.value == 32000.0f);
        i.enable = false;
        assert(FbOutputLimiter_Execute(&c, &i, &s, &o) == FB_OUTPUT_LIMITER_DISABLED);
        assert(s.precise_value == held);
        i.enable = true; i.dt_seconds = 0.0f;
        assert(FbOutputLimiter_Execute(&c, &i, &s, &o) == FB_OUTPUT_LIMITER_OK);
        assert(s.precise_value == held);
        i.dt_seconds = 0.01f; c.rise_rate = 0.0f;
        assert(FbOutputLimiter_Execute(&c, &i, &s, &o) == FB_OUTPUT_LIMITER_OK);
        assert(s.precise_value == held);
        c.upper_limit = 32000.0f;
        assert(FbOutputLimiter_Execute(&c, &i, &s, &o) == FB_OUTPUT_LIMITER_OK);
        assert(s.precise_value == 32000.0 && o.amplitude_limited);
    }

    printf("FbOutputLimiter host tests passed (25 cases); sizes: config=%lu input=%lu state=%lu output=%lu\n",
           (unsigned long)sizeof(c), (unsigned long)sizeof(i),
           (unsigned long)sizeof(s), (unsigned long)sizeof(o));
    return 0;
}
