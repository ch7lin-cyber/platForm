#ifndef TC_L_TABLE_H
#define TC_L_TABLE_H

#include "PiecewiseLinearTable.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TC_L_TABLE_COMPLETE (1U)

#define TC_L_MIN_MILLICELSIUS   (-200000L)
#define TC_L_MAX_MILLICELSIUS   (850000L)
#define TC_L_EXTENDED_MARGIN_MC (20000L)

#define TC_L_MEASUREMENT_SEGMENT_COUNT (55U)
#define TC_L_MEASUREMENT_BOUNDARY_COUNT (TC_L_MEASUREMENT_SEGMENT_COUNT + 1U)
#define TC_L_INPUT_SHIFT_UV (8750L)
#define TC_L_MEASUREMENT_COEFFICIENT_SCALE (100UL)

#define TC_L_CJC_MIN_MILLICELSIUS (-20000L)
#define TC_L_CJC_MAX_MILLICELSIUS (109000L)
#define TC_L_CJC_INTERVAL_MC (10000L)
#define TC_L_CJC_SEGMENT_COUNT (13U)
#define TC_L_CJC_INPUT_SHIFT_DECICELSIUS (200L)
#define TC_L_CJC_COEFFICIENT_SCALE (1000UL)

const PiecewiseLinearTable_t *TcLTable_GetMeasurementTable(void);
const PiecewiseLinearTable_t *TcLTable_GetCjcTable(void);
bool TcLTable_IsReady(void);

#ifdef __cplusplus
}
#endif

#endif /* TC_L_TABLE_H */

