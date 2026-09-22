#ifndef TC_B_TABLE_H
#define TC_B_TABLE_H

#include "PiecewiseLinearTable.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TC_B_TABLE_COMPLETE (0U)

#define TC_B_MIN_MILLICELSIUS   (100000L)
#define TC_B_MAX_MILLICELSIUS   (1800000L)
#define TC_B_EXTENDED_MARGIN_MC (20000L)

#define TC_B_MEASUREMENT_SEGMENT_COUNT (92U)
#define TC_B_MEASUREMENT_BOUNDARY_COUNT (TC_B_MEASUREMENT_SEGMENT_COUNT + 1U)
#define TC_B_INPUT_SHIFT_UV (100L)
#define TC_B_MEASUREMENT_COEFFICIENT_SCALE (100UL)

#define TC_B_CJC_MIN_MILLICELSIUS (-20000L)
#define TC_B_CJC_MAX_MILLICELSIUS (109000L)
#define TC_B_CJC_INTERVAL_MC (10000L)
#define TC_B_CJC_SEGMENT_COUNT (13U)
#define TC_B_CJC_INPUT_SHIFT_DECICELSIUS (200L)
#define TC_B_CJC_COEFFICIENT_SCALE (1000UL)

const PiecewiseLinearTable_t *TcBTable_GetMeasurementTable(void);
const PiecewiseLinearTable_t *TcBTable_GetCjcTable(void);
bool TcBTable_IsReady(void);

#ifdef __cplusplus
}
#endif

#endif /* TC_B_TABLE_H */

