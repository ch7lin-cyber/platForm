#include "FbLinearization.h"

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>

static unsigned cases;

#define CHECK(id, expression) do { \
    assert(expression); \
    ++cases; \
    printf("%s PASS\n", id); \
} while (0)

static bool Success(const FbLinearizationConfig *config, float value,
                    float expected, FbLinearizationRange range)
{
    FbLinearizationInput input = {value, true};
    FbLinearizationOutput output;
    FbLinearizationStatus status = FbLinearization_Execute(config, &input,
                                                          &output);
    return status == FB_LINEARIZATION_OK && output.status == status &&
           output.valid && output.value == expected && output.range == range &&
           isfinite(output.value);
}

static bool Failure(const FbLinearizationConfig *config,
                    const FbLinearizationInput *input,
                    FbLinearizationStatus expected)
{
    FbLinearizationOutput output = {99.0F, FB_LINEARIZATION_RANGE_ABOVE,
                                    true, FB_LINEARIZATION_OK};
    FbLinearizationStatus status = FbLinearization_Execute(config, input,
                                                          &output);
    return status == expected && output.status == status && !output.valid &&
           output.value == 0.0F && !signbit(output.value) &&
           output.range == FB_LINEARIZATION_RANGE_UNKNOWN;
}

int main(void)
{
    const FbLinearizationPoint basic[] = {{0.0F, 10.0F}, {10.0F, 30.0F}};
    const FbLinearizationPoint varied[] = {
        {-10.0F, 40.0F}, {-3.0F, -5.0F}, {2.0F, 15.0F},
        {8.0F, 15.0F}, {20.0F, -9.0F}
    };
    FbLinearizationPoint mutable_table[] = {
        {0.0F, 0.0F}, {1.0F, 10.0F}, {2.0F, 0.0F}, {3.0F, 30.0F}
    };
    FbLinearizationConfig config = {basic, 2, FB_LINEARIZATION_CLAMP};
    FbLinearizationInput input = {0.5F, true};
    FbLinearizationOutput output;
    size_t index;

    CHECK("UT-01", Success(&config, 2.5F, 15.0F,
                            FB_LINEARIZATION_RANGE_WITHIN));
    CHECK("UT-02", Success(&config, 0.0F, 10.0F,
                            FB_LINEARIZATION_RANGE_WITHIN));
    CHECK("UT-03", Success(&config, 10.0F, 30.0F,
                            FB_LINEARIZATION_RANGE_WITHIN));
    config.table = varied;
    config.count = sizeof varied / sizeof varied[0];
    for (index = 0; index < config.count; ++index) {
        assert(Success(&config, varied[index].x, varied[index].y,
                       FB_LINEARIZATION_RANGE_WITHIN));
    }
    CHECK("UT-04", index == 5);
    CHECK("UT-05", Success(&config, -6.5F, 17.5F,
                            FB_LINEARIZATION_RANGE_WITHIN));
    CHECK("UT-06", Success(&config, 1.0F, 11.0F,
                            FB_LINEARIZATION_RANGE_WITHIN));
    CHECK("UT-07", Success(&config, 5.0F, 15.0F,
                            FB_LINEARIZATION_RANGE_WITHIN));
    CHECK("UT-08", Success(&config, 14.0F, 3.0F,
                            FB_LINEARIZATION_RANGE_WITHIN));
    CHECK("UT-09", Success(&config, -3.0F, -5.0F,
                            FB_LINEARIZATION_RANGE_WITHIN));
    CHECK("UT-10", Success(&config, -FLT_MAX, 40.0F,
                            FB_LINEARIZATION_RANGE_BELOW));
    CHECK("UT-11", Success(&config, FLT_MAX, -9.0F,
                            FB_LINEARIZATION_RANGE_ABOVE));
    config.policy = FB_LINEARIZATION_ERROR;
    input.value = -11.0F;
    CHECK("UT-12", Failure(&config, &input, FB_LINEARIZATION_OUT_OF_RANGE));
    input.value = 21.0F;
    CHECK("UT-13", Failure(&config, &input, FB_LINEARIZATION_OUT_OF_RANGE));
    CHECK("UT-14", Success(&config, -10.0F, 40.0F,
                            FB_LINEARIZATION_RANGE_WITHIN) &&
                   Success(&config, 20.0F, -9.0F,
                            FB_LINEARIZATION_RANGE_WITHIN));
    config.policy = (FbLinearizationPolicy)-1;
    CHECK("UT-15", Failure(&config, &input,
                            FB_LINEARIZATION_INVALID_CONFIGURATION));
    config.policy = (FbLinearizationPolicy)2;
    CHECK("UT-16", Failure(&config, &input,
                            FB_LINEARIZATION_INVALID_CONFIGURATION));
    config.policy = FB_LINEARIZATION_CLAMP;
    config.table = NULL;
    CHECK("UT-17", Failure(&config, &input,
                            FB_LINEARIZATION_INVALID_CONFIGURATION));
    config.table = basic;
    config.count = 0;
    CHECK("UT-18", Failure(&config, &input,
                            FB_LINEARIZATION_INVALID_CONFIGURATION));
    config.count = 1;
    CHECK("UT-19", Failure(&config, &input,
                            FB_LINEARIZATION_INVALID_CONFIGURATION));
    config.table = mutable_table;
    config.count = 4;
    mutable_table[2].x = 1.0F;
    CHECK("UT-20", Failure(&config, &input,
                            FB_LINEARIZATION_INVALID_CONFIGURATION));
    mutable_table[2].x = 0.5F;
    CHECK("UT-21", Failure(&config, &input,
                            FB_LINEARIZATION_INVALID_CONFIGURATION));
    mutable_table[0].x = -0.0F;
    mutable_table[1].x = 0.0F;
    CHECK("UT-22", Failure(&config, &input,
                            FB_LINEARIZATION_INVALID_CONFIGURATION));
    mutable_table[0].x = 0.0F;
    mutable_table[1].x = 1.0F;
    mutable_table[2].x = 2.0F;
    {
        const float bad[] = {NAN, INFINITY, -INFINITY};
        size_t bad_index;
        for (index = 0; index < config.count; ++index) {
            float saved = mutable_table[index].x;
            for (bad_index = 0; bad_index < 3; ++bad_index) {
                mutable_table[index].x = bad[bad_index];
                assert(Failure(&config, &input,
                               FB_LINEARIZATION_INVALID_CONFIGURATION));
            }
            mutable_table[index].x = saved;
        }
        CHECK("UT-23", index == 4);
        for (index = 0; index < config.count; ++index) {
            float saved = mutable_table[index].y;
            for (bad_index = 0; bad_index < 3; ++bad_index) {
                mutable_table[index].y = bad[bad_index];
                assert(Failure(&config, &input,
                               FB_LINEARIZATION_INVALID_CONFIGURATION));
            }
            mutable_table[index].y = saved;
        }
        CHECK("UT-24", index == 4);
    }
    mutable_table[3].y = NAN;
    input.value = -1.0F;
    CHECK("UT-25", Failure(&config, &input,
                            FB_LINEARIZATION_INVALID_CONFIGURATION));
    input.value = 4.0F;
    CHECK("UT-26", Failure(&config, &input,
                            FB_LINEARIZATION_INVALID_CONFIGURATION));
    config.policy = FB_LINEARIZATION_ERROR;
    CHECK("UT-27", Failure(&config, &input,
                            FB_LINEARIZATION_INVALID_CONFIGURATION));
    input.value = 0.0F;
    CHECK("UT-28", Failure(&config, &input,
                            FB_LINEARIZATION_INVALID_CONFIGURATION));
    input.value = 0.5F;
    CHECK("UT-29", Failure(&config, &input,
                            FB_LINEARIZATION_INVALID_CONFIGURATION));
    input.value = NAN;
    CHECK("UT-30", Failure(&config, &input,
                            FB_LINEARIZATION_INVALID_CONFIGURATION));
    mutable_table[3].y = 30.0F;
    {
        const float bad[] = {NAN, INFINITY, -INFINITY};
        for (index = 0; index < 3; ++index) {
            input.value = bad[index];
            assert(Failure(&config, &input, FB_LINEARIZATION_INVALID_INPUT));
        }
        CHECK("UT-31", index == 3);
    }
    input.enable = false;
    config.table = NULL;
    config.count = (size_t)-1;
    config.policy = (FbLinearizationPolicy)-1;
    CHECK("UT-32", Failure(&config, &input, FB_LINEARIZATION_DISABLED));
    config.table = mutable_table;
    config.count = 4;
    mutable_table[3].x = NAN;
    mutable_table[0].y = INFINITY;
    CHECK("UT-33", Failure(&config, &input, FB_LINEARIZATION_DISABLED));
    input.enable = true;
    CHECK("UT-34", Failure(NULL, &input, FB_LINEARIZATION_INVALID_ARGUMENT));
    CHECK("UT-35", Failure(&config, NULL, FB_LINEARIZATION_INVALID_ARGUMENT));
    CHECK("UT-36", FbLinearization_Execute(&config, &input, NULL) ==
                   FB_LINEARIZATION_INVALID_ARGUMENT);
    input.enable = false;
    CHECK("UT-37", Failure(NULL, &input, FB_LINEARIZATION_INVALID_ARGUMENT));
    CHECK("UT-38", FbLinearization_Execute(NULL, NULL, NULL) ==
                   FB_LINEARIZATION_INVALID_ARGUMENT);
    input.enable = true;
    config.table = basic;
    config.count = 2;
    config.policy = FB_LINEARIZATION_CLAMP;
    CHECK("UT-39", Success(&config, 2.5F, 15.0F,
                            FB_LINEARIZATION_RANGE_WITHIN));
    {
        FbLinearizationPoint extreme[] = {
            {-FLT_MAX, -100.0F}, {FLT_MAX, 100.0F}
        };
        config.table = extreme;
        CHECK("UT-40", Success(&config, 0.0F, 0.0F,
                                FB_LINEARIZATION_RANGE_WITHIN));
        extreme[0].x = -1.0F;
        extreme[1].x = 1.0F;
        extreme[0].y = -FLT_MAX;
        extreme[1].y = FLT_MAX;
        CHECK("UT-41", Success(&config, 0.0F, 0.0F,
                                FB_LINEARIZATION_RANGE_WITHIN));
        extreme[0].x = -FLT_MAX;
        extreme[1].x = FLT_MAX;
        CHECK("UT-42", Success(&config, 0.0F, 0.0F,
                                FB_LINEARIZATION_RANGE_WITHIN));
        CHECK("UT-43", Success(&config, -FLT_MAX * 0.5F, -FLT_MAX * 0.5F,
                                FB_LINEARIZATION_RANGE_WITHIN) &&
                       Success(&config, FLT_MAX * 0.5F, FLT_MAX * 0.5F,
                                FB_LINEARIZATION_RANGE_WITHIN));
        extreme[0].y = FLT_MAX;
        extreme[1].y = -FLT_MAX;
        CHECK("UT-44", Success(&config, 0.0F, 0.0F,
                                FB_LINEARIZATION_RANGE_WITHIN) &&
                       Success(&config, FLT_MAX * 0.5F, -FLT_MAX * 0.5F,
                                FB_LINEARIZATION_RANGE_WITHIN));
        extreme[1].y = FLT_MAX;
        CHECK("UT-45", Success(&config, 0.0F, FLT_MAX,
                                FB_LINEARIZATION_RANGE_WITHIN));
        extreme[0].y = -FLT_MAX;
        extreme[1].y = -FLT_MAX;
        CHECK("UT-46", Success(&config, 0.0F, -FLT_MAX,
                                FB_LINEARIZATION_RANGE_WITHIN));
    }
    {
        FbLinearizationPoint edge[] = {
            {0.0F, nextafterf(FLT_MAX, 0.0F)}, {1.0F, FLT_MAX}
        };
        config.table = edge;
        for (index = 0; index <= 1024; ++index) {
            input.value = (float)index / 1024.0F;
            assert(FbLinearization_Execute(&config, &input, &output) ==
                   FB_LINEARIZATION_OK && output.valid &&
                   output.status == FB_LINEARIZATION_OK &&
                   output.range == FB_LINEARIZATION_RANGE_WITHIN &&
                   isfinite(output.value) && output.value >= edge[0].y &&
                   output.value <= edge[1].y);
        }
        CHECK("UT-47", index == 1025);
        edge[0].y = -FLT_MAX;
        edge[1].y = nextafterf(-FLT_MAX, 0.0F);
        for (index = 0; index <= 1024; ++index) {
            input.value = (float)index / 1024.0F;
            assert(FbLinearization_Execute(&config, &input, &output) ==
                   FB_LINEARIZATION_OK && output.valid &&
                   output.status == FB_LINEARIZATION_OK &&
                   output.range == FB_LINEARIZATION_RANGE_WITHIN &&
                   isfinite(output.value) && output.value >= edge[0].y &&
                   output.value <= edge[1].y);
        }
        CHECK("UT-48", index == 1025);
    }
    {
        float tiny = nextafterf(0.0F, 1.0F);
        FbLinearizationPoint small[] = {{0.0F, 0.0F}, {4.0F * tiny, 4.0F}};
        config.table = small;
        CHECK("UT-49", Success(&config, tiny, 1.0F,
                                FB_LINEARIZATION_RANGE_WITHIN));
        small[1].x = 4.0F;
        small[1].y = 4.0F * tiny;
        CHECK("UT-50", Success(&config, 1.0F, tiny,
                                FB_LINEARIZATION_RANGE_WITHIN));
        small[0].x = 1.0F;
        small[0].y = -FLT_MAX;
        small[1].x = nextafterf(1.0F, 2.0F);
        small[1].y = FLT_MAX;
        CHECK("UT-51", Success(&config, small[0].x, -FLT_MAX,
                                FB_LINEARIZATION_RANGE_WITHIN) &&
                       Success(&config, small[1].x, FLT_MAX,
                                FB_LINEARIZATION_RANGE_WITHIN));
    }
    {
        const FbLinearizationPoint signed_zero[] = {
            {0.0F, -0.0F}, {1.0F, -0.0F}, {2.0F, -0.0F}
        };
        config.table = signed_zero;
        config.count = 3;
        for (index = 0; index < config.count; ++index) {
            input.value = signed_zero[index].x;
            assert(FbLinearization_Execute(&config, &input, &output) ==
                   FB_LINEARIZATION_OK && output.valid &&
                   output.value == 0.0F && signbit(output.value));
        }
        CHECK("UT-52", index == 3);
        input.value = -0.0F;
        CHECK("UT-53", FbLinearization_Execute(&config, &input, &output) ==
                       FB_LINEARIZATION_OK && output.valid &&
                       output.range == FB_LINEARIZATION_RANGE_WITHIN &&
                       output.value == 0.0F && signbit(output.value));
    }
    {
        FbLinearizationPoint changed[] = {{0.0F, 0.0F}, {1.0F, 10.0F}};
        config.table = changed;
        config.count = 2;
        assert(Success(&config, 0.5F, 5.0F, FB_LINEARIZATION_RANGE_WITHIN));
        changed[1].y = 20.0F;
        assert(Success(&config, 0.5F, 10.0F, FB_LINEARIZATION_RANGE_WITHIN));
        changed[1].x = 0.0F;
        input.value = 0.5F;
        CHECK("UT-54", Failure(&config, &input,
                                FB_LINEARIZATION_INVALID_CONFIGURATION));
    }
    {
        FbLinearizationConfig other = {basic, 2, FB_LINEARIZATION_ERROR};
        config.table = varied;
        config.count = 5;
        for (index = 0; index < 100; ++index) {
            assert(Success(&config, 5.0F, 15.0F,
                           FB_LINEARIZATION_RANGE_WITHIN));
            assert(Success(&other, 5.0F, 20.0F,
                           FB_LINEARIZATION_RANGE_WITHIN));
        }
        CHECK("UT-55", index == 100);
    }
    {
        const FbLinearizationPoint sweep[] = {
            {-2.0F, -FLT_MAX}, {0.0F, FLT_MAX}, {1.0F, -FLT_MAX},
            {2.0F, FLT_MAX}, {3.0F, FLT_MAX}
        };
        size_t segment;
        config.table = sweep;
        config.count = 5;
        for (segment = 0; segment + 1 < config.count; ++segment) {
            float low = sweep[segment].y < sweep[segment + 1].y ?
                        sweep[segment].y : sweep[segment + 1].y;
            float high = sweep[segment].y > sweep[segment + 1].y ?
                         sweep[segment].y : sweep[segment + 1].y;
            for (index = 0; index <= 1024; ++index) {
                input.value = sweep[segment].x +
                              (sweep[segment + 1].x - sweep[segment].x) *
                              ((float)index / 1024.0F);
                assert(FbLinearization_Execute(&config, &input, &output) ==
                       FB_LINEARIZATION_OK && output.valid &&
                       output.status == FB_LINEARIZATION_OK &&
                       output.range == FB_LINEARIZATION_RANGE_WITHIN &&
                       isfinite(output.value) && output.value >= low &&
                       output.value <= high);
            }
        }
        CHECK("UT-56", segment == 4);
    }
    {
        FbLinearizationPoint affine[] = {
            {-FLT_MAX, -FLT_MAX}, {FLT_MAX, FLT_MAX}
        };
        const float small[] = {1.0F, -1.0F, FLT_MIN, -FLT_MIN,
                               nextafterf(0.0F, 1.0F),
                               -nextafterf(0.0F, 1.0F)};
        config.table = affine;
        config.count = 2;
        for (index = 0; index < sizeof small / sizeof small[0]; ++index) {
            assert(Success(&config, small[index], small[index],
                           FB_LINEARIZATION_RANGE_WITHIN));
        }
        CHECK("UT-57", index == 6);
        affine[0].y = FLT_MAX;
        affine[1].y = -FLT_MAX;
        for (index = 0; index < sizeof small / sizeof small[0]; ++index) {
            assert(Success(&config, small[index], -small[index],
                           FB_LINEARIZATION_RANGE_WITHIN));
        }
        CHECK("UT-58", index == 6);
        affine[0].x = -FLT_MAX * 0.5F;
        affine[0].y = -FLT_MAX;
        affine[1].x = FLT_MAX * 0.25F;
        affine[1].y = FLT_MAX * 0.5F;
        for (index = 0; index < sizeof small / sizeof small[0]; ++index) {
            assert(Success(&config, small[index], small[index] * 2.0F,
                           FB_LINEARIZATION_RANGE_WITHIN));
        }
        CHECK("UT-59", index == 6);
    }
    printf("FbLinearization: %u cases passed; sizeof Point=%zu Config=%zu "
           "Input=%zu Output=%zu\n", cases, sizeof(FbLinearizationPoint),
           sizeof config, sizeof input, sizeof output);
    return 0;
}
