#include "tc_b_table.h"

#include <stddef.h>

#if TC_B_TABLE_COMPLETE

#if (TC_B_MEASUREMENT_SEGMENT_COUNT == 0U)
#error "The B measurement segment count must not be zero"
#endif

/* Values are input_uV + TC_B_INPUT_SHIFT_UV. */
static const int32_t s_tc_b_measurement_boundaries_shifted_uv
    [TC_B_MEASUREMENT_BOUNDARY_COUNT] =
{
    117, 133, 153, 178, 207,
    241, 278, 320, 367, 417, 472, 531, 594, 661, 732,
    807, 887, 970, 1057, 1148, 1243, 1342, 1444, 1551, 1661,
    1775, 1892, 2013, 2137, 2265, 2396, 2531, 2669, 2810, 2954,
    3102, 3254, 3408, 3566, 3726, 3890, 4057, 4227, 4399, 4575,
    4753, 4934, 5118, 5305, 5494, 5685, 5880, 6076, 6275, 6477,
    6680, 6886, 7095, 7305, 7517, 7732, 7948, 8166, 8386, 8608,
    8831, 9056, 9282, 9510, 9739, 9968, 10199, 10431, 10663, 10896,
    11129, 11363, 11597, 11831, 12065, 12299, 12533, 12766, 12998, 13230,
    13461, 13691, 13920,
};

/*
 * B-type reference EMF in 0.001 uV. Values below 0 C extrapolate the
 * low-range ITS-90 polynomial to cover the configured CJC input range.
 */
static const int32_t
    s_tc_b_cjc_boundaries_milli_uv[TC_B_CJC_BOUNDARY_COUNT] =
{
    7303, 3057, 0, -1876, -2579, -2116, -495,
    2278, 6197, 11254, 17445, 24764, 33204, 42762,
};

static PiecewiseLinearSegment_t
    s_tc_b_measurement_segments[TC_B_MEASUREMENT_SEGMENT_COUNT];
static PiecewiseLinearSegment_t
    s_tc_b_cjc_segments[TC_B_CJC_SEGMENT_COUNT];
static bool s_tables_initialized;

static const PiecewiseLinearTable_t s_tc_b_measurement_table =
{
    s_tc_b_measurement_segments,
    TC_B_MEASUREMENT_SEGMENT_COUNT,
    TC_B_MEASUREMENT_COEFFICIENT_SCALE
};

static const PiecewiseLinearTable_t s_tc_b_cjc_table =
{
    s_tc_b_cjc_segments,
    TC_B_CJC_SEGMENT_COUNT,
    TC_B_CJC_COEFFICIENT_SCALE
};

static int32_t DivideRounded(int64_t numerator, int32_t denominator)
{
    if (numerator >= 0LL)
    {
        return (int32_t)((numerator + (denominator / 2)) / denominator);
    }
    return (int32_t)((numerator - (denominator / 2)) / denominator);
}

static void InitializeTables(void)
{
    uint16_t index;

    if (s_tables_initialized)
    {
        return;
    }

    for (index = 0U; index < TC_B_MEASUREMENT_SEGMENT_COUNT; index++)
    {
        int32_t shifted_min =
            s_tc_b_measurement_boundaries_shifted_uv[index];
        int32_t delta_uv =
            s_tc_b_measurement_boundaries_shifted_uv[index + 1U] -
            shifted_min;
        int32_t expected_min =
            (TC_B_MIN_MILLICELSIUS - TC_B_EXTENDED_MARGIN_MC) +
            ((int32_t)index * 20000L);
        int32_t slope = DivideRounded(
            20000LL * TC_B_MEASUREMENT_COEFFICIENT_SCALE,
            delta_uv);
        s_tc_b_measurement_segments[index].x_min =
            shifted_min - TC_B_INPUT_SHIFT_UV;
        s_tc_b_measurement_segments[index].x_max =
            s_tc_b_measurement_boundaries_shifted_uv[index + 1U] -
            TC_B_INPUT_SHIFT_UV;
        s_tc_b_measurement_segments[index].slope = slope;
        s_tc_b_measurement_segments[index].intercept =
            ((int64_t)expected_min *
             TC_B_MEASUREMENT_COEFFICIENT_SCALE) -
            ((int64_t)slope *
             s_tc_b_measurement_segments[index].x_min);
    }

    for (index = 0U; index < TC_B_CJC_SEGMENT_COUNT; index++)
    {
        int64_t delta_scaled =
            ((int64_t)s_tc_b_cjc_boundaries_milli_uv[index + 1U] -
             s_tc_b_cjc_boundaries_milli_uv[index]) *
            TC_B_CJC_COEFFICIENT_SCALE;
        int32_t slope = DivideRounded(
            delta_scaled,
            (int32_t)(TC_B_CJC_INTERVAL_MC * 1000L));
        s_tc_b_cjc_segments[index].x_min =
            TC_B_CJC_MIN_MILLICELSIUS +
            ((int32_t)index * TC_B_CJC_INTERVAL_MC);
        s_tc_b_cjc_segments[index].x_max =
            s_tc_b_cjc_segments[index].x_min + TC_B_CJC_INTERVAL_MC;
        s_tc_b_cjc_segments[index].slope = slope;
        s_tc_b_cjc_segments[index].intercept =
            ((int64_t)s_tc_b_cjc_boundaries_milli_uv[index] * 1000LL) -
            ((int64_t)slope * s_tc_b_cjc_segments[index].x_min);
    }

    s_tables_initialized = true;
}

const PiecewiseLinearTable_t *TcBTable_GetMeasurementTable(void)
{
    InitializeTables();
    return &s_tc_b_measurement_table;
}

const PiecewiseLinearTable_t *TcBTable_GetCjcTable(void)
{
    InitializeTables();
    return &s_tc_b_cjc_table;
}

bool TcBTable_IsReady(void)
{
    InitializeTables();
    return PiecewiseLinearTable_IsValid(&s_tc_b_measurement_table) &&
           PiecewiseLinearTable_IsValid(&s_tc_b_cjc_table);
}

#else

const PiecewiseLinearTable_t *TcBTable_GetMeasurementTable(void)
{
    return NULL;
}

const PiecewiseLinearTable_t *TcBTable_GetCjcTable(void)
{
    return NULL;
}

bool TcBTable_IsReady(void)
{
    return false;
}

#endif /* TC_B_TABLE_COMPLETE */
