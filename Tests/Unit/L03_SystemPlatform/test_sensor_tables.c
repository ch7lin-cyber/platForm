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
    assert(!TcBTable_IsReady());
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
    TestKnownKTypePoints();
    TestKnownCu50Points();
    return 0;
}
