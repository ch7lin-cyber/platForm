#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "rtd_cu50_table.h"
#include "rtd_jpt100_table.h"
#include "rtd_ni120_table.h"
#include "rtd_pt100_table.h"
#include "rtd_pt1000_table.h"
#include "tc_b_table.h"
#include "tc_c_table.h"
#include "tc_d_table.h"
#include "tc_e_table.h"
#include "tc_j_table.h"
#include "tc_k_table.h"
#include "tc_l_table.h"
#include "tc_n_table.h"
#include "tc_r_table.h"
#include "tc_s_table.h"
#include "tc_t_table.h"
#include "tc_txk_table.h"
#include "tc_u_table.h"

static void TestTableReadiness(void)
{
    assert(TcBTable_IsReady());
    assert(TcCTable_IsReady());
    assert(TcDTable_IsReady());
    assert(TcETable_IsReady());
    assert(TcJTable_IsReady());
    assert(TcKTable_IsReady());
    assert(TcLTable_IsReady());
    assert(TcNTable_IsReady());
    assert(TcRTable_IsReady());
    assert(TcSTable_IsReady());
    assert(TcTTable_IsReady());
    assert(TcTxkTable_IsReady());
    assert(TcUTable_IsReady());
    assert(RtdCu50Table_IsReady());
    assert(RtdJpt100Table_IsReady());
    assert(RtdNi120Table_IsReady());
    assert(RtdPt100Table_IsReady());
    assert(RtdPt1000Table_IsReady());
}

static void TestKnownBTypePoints(void)
{
    const PiecewiseLinearTable_t *measurement =
        TcBTable_GetMeasurementTable();
    const PiecewiseLinearTable_t *cjc = TcBTable_GetCjcTable();
    int32_t value;

    assert(measurement != NULL);
    assert(cjc != NULL);

    /* Rounded NIST ITS-90 B-type EMF reference points. */
    assert(PiecewiseLinearTable_Evaluate(measurement, 17L, &value));
    assert(value == 80000L);
    assert(PiecewiseLinearTable_Evaluate(measurement, 33L, &value));
    assert(value == 100000L);
    assert(PiecewiseLinearTable_Evaluate(measurement, 1242L, &value));
    assert(value >= 499900L);
    assert(value <= 500100L);
    assert(PiecewiseLinearTable_Evaluate(measurement, 4834L, &value));
    assert(value >= 999900L);
    assert(value <= 1000100L);
    assert(PiecewiseLinearTable_Evaluate(measurement, 10099L, &value));
    assert(value >= 1499900L);
    assert(value <= 1500100L);
    assert(PiecewiseLinearTable_Evaluate(measurement, 13591L, &value));
    assert(value >= 1799900L);
    assert(value <= 1800100L);
    assert(!PiecewiseLinearTable_Evaluate(measurement, 16L, &value));
    assert(!PiecewiseLinearTable_Evaluate(measurement, 13821L, &value));

    assert(PiecewiseLinearTable_Evaluate(cjc, -20000L, &value));
    assert(value == 7L);
    assert(PiecewiseLinearTable_Evaluate(cjc, 0L, &value));
    assert(value == 0L);
    assert(PiecewiseLinearTable_Evaluate(cjc, 25000L, &value));
    assert(value == -2L);
    assert(PiecewiseLinearTable_Evaluate(cjc, 100000L, &value));
    assert(value == 33L);
}

static void TestBTypeMeasurementCoverage(void)
{
    const PiecewiseLinearTable_t *table =
        TcBTable_GetMeasurementTable();
    uint16_t index;

    assert(table != NULL);
    assert(table->segment_count == TC_B_MEASUREMENT_SEGMENT_COUNT);
    assert(table->segments[0].x_min == 17L);
    assert(table->segments[table->segment_count - 1U].x_max == 13820L);

    for (index = 0U; index < table->segment_count; index++)
    {
        const PiecewiseLinearSegment_t *segment = &table->segments[index];
        int32_t expected_min = 80000L + ((int32_t)index * 20000L);
        int32_t expected_max = expected_min + 20000L;
        int32_t value;

        assert(segment->x_min < segment->x_max);
        if (index > 0U)
        {
            assert(segment->x_min == table->segments[index - 1U].x_max);
        }
        assert(PiecewiseLinearTable_Evaluate(
            table, segment->x_min, &value));
        assert(value >= (expected_min - 100L));
        assert(value <= (expected_min + 100L));
        assert(PiecewiseLinearTable_Evaluate(
            table, segment->x_max, &value));
        assert(value >= (expected_max - 100L));
        assert(value <= (expected_max + 100L));
    }
}

static void TestKnownKTypePoints(void)
{
    const PiecewiseLinearTable_t *table = TcKTable_GetMeasurementTable();
    int32_t temperature_mc;

    assert(table != NULL);
    assert(PiecewiseLinearTable_Evaluate(table, -6037L, &temperature_mc));
    assert(temperature_mc == -210550L);
    assert(PiecewiseLinearTable_Evaluate(table, 6540L, &temperature_mc));
    assert(temperature_mc == 159988L);
    assert(PiecewiseLinearTable_Evaluate(table, 34093L, &temperature_mc));
    assert(temperature_mc == 819983L);
}

static void TestKnownKTypeCjcPoint(void)
{
    const PiecewiseLinearTable_t *table = TcKTable_GetCjcTable();
    int32_t microvolts;

    assert(table != NULL);
    assert(PiecewiseLinearTable_Evaluate(table, 25000L, &microvolts));
    /* K type at 25 C is approximately 1.000 mV. */
    assert(microvolts >= 995L);
    assert(microvolts <= 1006L);
}

static void TestKnownCu50Points(void)
{
    const PiecewiseLinearTable_t *table =
        RtdCu50Table_GetMeasurementTable();
    int32_t temperature_mc;

    assert(table != NULL);
    assert(PiecewiseLinearTable_Evaluate(table, 35019L, &temperature_mc));
    assert(temperature_mc == -69999L);
    assert(PiecewiseLinearTable_Evaluate(table, 50000L, &temperature_mc));
    assert(temperature_mc == 0L);
    assert(PiecewiseLinearTable_Evaluate(table, 86380L, &temperature_mc));
    assert(temperature_mc == 169993L);
}

int main(void)
{
    TestTableReadiness();
    TestKnownBTypePoints();
    TestBTypeMeasurementCoverage();
    TestKnownKTypePoints();
    TestKnownKTypeCjcPoint();
    TestKnownCu50Points();
    return 0;
}
