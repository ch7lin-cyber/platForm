#ifndef TC_TXK_TABLE_H
#define TC_TXK_TABLE_H

#include "PiecewiseLinearTable.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TC_TXK_TABLE_COMPLETE (1U)

#define TC_TXK_MIN_MILLICELSIUS   (-150000L)
#define TC_TXK_MAX_MILLICELSIUS   (800000L)
#define TC_TXK_EXTENDED_MARGIN_MC (20000L)

#define TC_TXK_MEASUREMENT_SEGMENT_COUNT (50U)
#define TC_TXK_MEASUREMENT_BOUNDARY_COUNT (TC_TXK_MEASUREMENT_SEGMENT_COUNT + 1U)
#define TC_TXK_INPUT_SHIFT_UV (9800L)
#define TC_TXK_MEASUREMENT_COEFFICIENT_SCALE (100UL)

#define TC_TXK_CJC_MIN_MILLICELSIUS (-20000L)
#define TC_TXK_CJC_MAX_MILLICELSIUS (109000L)
#define TC_TXK_CJC_INTERVAL_MC (10000L)
#define TC_TXK_CJC_SEGMENT_COUNT (13U)
#define TC_TXK_CJC_INPUT_SHIFT_DECICELSIUS (200L)
#define TC_TXK_CJC_COEFFICIENT_SCALE (1000UL)

const PiecewiseLinearTable_t *TcTxkTable_GetMeasurementTable(void);
const PiecewiseLinearTable_t *TcTxkTable_GetCjcTable(void);
bool TcTxkTable_IsReady(void);

#ifdef __cplusplus
}
#endif

#endif /* TC_TXK_TABLE_H */

