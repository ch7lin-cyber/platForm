#include "tc_n_table.h"

#include <stddef.h>

#if TC_N_TABLE_COMPLETE

#if (TC_N_MEASUREMENT_SEGMENT_COUNT == 0U)
#error "The N measurement segment count must not be zero"
#endif

typedef struct
{
    int32_t slope;
    int32_t intercept_millicelsius;
} TcNMeasurementCoefficient_t;

typedef struct
{
    int32_t slope;
    int32_t intercept_deci_microvolts;
} TcNCjcCoefficient_t;

/* Values are input_uV + TC_N_INPUT_SHIFT_UV. */
static const int32_t s_tc_n_measurement_boundaries_shifted_uv
    [TC_N_MEASUREMENT_BOUNDARY_COUNT] =
{
    38, 210, 434, 709, 1029, 1392, 1793, 2228, 2691, 3177,
    3682, 4200, 4725, 5265, 5819, 6389, 6974, 7574, 8189, 8818,
    9459, 10113, 10779, 11455, 12141, 12837, 13541, 14254, 14974, 15701,
    16434, 17174, 17919, 18669, 19425, 20184, 20948, 21715, 22486, 23259,
    24035, 24813, 25593, 26375, 27158, 27942, 28727, 29512, 30298, 31083,
    31869, 32655, 33439, 34224, 35007, 35790, 36571, 37351, 38130, 38907,
    39682, 40456, 41227, 41995, 42762, 43526, 44287, 45045, 45800, 46552,
    47301, 48046, 48788, 49526, 50260, 50989, 51713,
};

/* temperature_mC = slope * shifted_uV / scale + intercept_mC. */
static const TcNMeasurementCoefficient_t
    s_tc_n_measurement_coefficients[TC_N_MEASUREMENT_SEGMENT_COUNT] =
{
    {11628, -224001}, {8929, -218447}, {7273, -211338}, {6250, -204157}, {5510, -196566},
    {4988, -189298}, {4598, -182327}, {4320, -176168}, {4115, -170673}, {3960, -165764},
    {3861, -162130}, {3810, -159988}, {3704, -154977}, {3610, -150019}, {3509, -144145},
    {3419, -138402}, {3333, -132409}, {3252, -126282}, {3180, -120378}, {3120, -115103},
    {3058, -109239}, {3003, -103669}, {2959, -98925}, {2915, -93895}, {2874, -88916},
    {2841, -84682}, {2805, -79809}, {2778, -75962}, {2751, -71914}, {2729, -68464},
    {2703, -64199}, {2685, -61111}, {2667, -57901}, {2646, -53984}, {2635, -51837},
    {2618, -48413}, {2608, -46320}, {2594, -43279}, {2587, -41703}, {2577, -39364},
    {2571, -37937}, {2564, -36205}, {2558, -34668}, {2554, -33608}, {2551, -32796},
    {2548, -31956}, {2548, -31958}, {2545, -31079}, {2548, -31997}, {2545, -31071},
    {2545, -31064}, {2551, -33036}, {2548, -32030}, {2554, -34082}, {2554, -34081},
    {2561, -36585}, {2564, -37680}, {2567, -38805}, {2574, -41470}, {2581, -44198},
    {2584, -45384}, {2594, -49424}, {2604, -53556}, {2608, -55240}, {2618, -59506},
    {2628, -63861}, {2639, -68733}, {2649, -73242}, {2660, -78288}, {2669, -82476},
    {2685, -90043}, {2695, -94855}, {2710, -102172}, {2725, -109602}, {2743, -118650},
    {2762, -128333},
};

/* cjc_deci_uV = slope * shifted_temperature_deci_C + intercept. */
static const TcNCjcCoefficient_t
    s_tc_n_cjc_coefficients[TC_N_CJC_SEGMENT_COUNT] =
{
    {258, -51840}, {260, -52050}, {261, -52235}, {264, -53120}, {268, -54730},
    {272, -56760}, {275, -58550}, {279, -61335}, {283, -64545}, {287, -68145},
    {291, -72145}, {294, -75450}, {298, -80210},
};

static PiecewiseLinearSegment_t
    s_tc_n_measurement_segments[TC_N_MEASUREMENT_SEGMENT_COUNT];
static PiecewiseLinearSegment_t
    s_tc_n_cjc_segments[TC_N_CJC_SEGMENT_COUNT];
static bool s_tables_initialized;

static const PiecewiseLinearTable_t s_tc_n_measurement_table =
{
    s_tc_n_measurement_segments,
    TC_N_MEASUREMENT_SEGMENT_COUNT,
    TC_N_MEASUREMENT_COEFFICIENT_SCALE
};

static const PiecewiseLinearTable_t s_tc_n_cjc_table =
{
    s_tc_n_cjc_segments,
    TC_N_CJC_SEGMENT_COUNT,
    TC_N_CJC_COEFFICIENT_SCALE
};

static void InitializeTables(void)
{
    uint16_t index;

    if (s_tables_initialized)
    {
        return;
    }

    for (index = 0U; index < TC_N_MEASUREMENT_SEGMENT_COUNT; index++)
    {
        int32_t slope = s_tc_n_measurement_coefficients[index].slope;
        s_tc_n_measurement_segments[index].x_min =
            s_tc_n_measurement_boundaries_shifted_uv[index] -
            TC_N_INPUT_SHIFT_UV;
        s_tc_n_measurement_segments[index].x_max =
            s_tc_n_measurement_boundaries_shifted_uv[index + 1U] -
            TC_N_INPUT_SHIFT_UV;
        s_tc_n_measurement_segments[index].slope = slope;
        s_tc_n_measurement_segments[index].intercept =
            ((int64_t)slope * TC_N_INPUT_SHIFT_UV) +
            ((int64_t)s_tc_n_measurement_coefficients[index]
                 .intercept_millicelsius *
             TC_N_MEASUREMENT_COEFFICIENT_SCALE);
    }

    for (index = 0U; index < TC_N_CJC_SEGMENT_COUNT; index++)
    {
        int32_t slope = s_tc_n_cjc_coefficients[index].slope;
        s_tc_n_cjc_segments[index].x_min =
            TC_N_CJC_MIN_MILLICELSIUS +
            ((int32_t)index * TC_N_CJC_INTERVAL_MC);
        s_tc_n_cjc_segments[index].x_max =
            s_tc_n_cjc_segments[index].x_min + TC_N_CJC_INTERVAL_MC;
        s_tc_n_cjc_segments[index].slope = slope;
        s_tc_n_cjc_segments[index].intercept =
            (((int64_t)slope * TC_N_CJC_INPUT_SHIFT_DECICELSIUS) +
             s_tc_n_cjc_coefficients[index].intercept_deci_microvolts) *
            100LL;
    }

    s_tables_initialized = true;
}

const PiecewiseLinearTable_t *TcNTable_GetMeasurementTable(void)
{
    InitializeTables();
    return &s_tc_n_measurement_table;
}

const PiecewiseLinearTable_t *TcNTable_GetCjcTable(void)
{
    InitializeTables();
    return &s_tc_n_cjc_table;
}

bool TcNTable_IsReady(void)
{
    InitializeTables();
    return PiecewiseLinearTable_IsValid(&s_tc_n_measurement_table) &&
           PiecewiseLinearTable_IsValid(&s_tc_n_cjc_table);
}

#else

const PiecewiseLinearTable_t *TcNTable_GetMeasurementTable(void)
{
    return NULL;
}

const PiecewiseLinearTable_t *TcNTable_GetCjcTable(void)
{
    return NULL;
}

bool TcNTable_IsReady(void)
{
    return false;
}

#endif /* TC_N_TABLE_COMPLETE */

