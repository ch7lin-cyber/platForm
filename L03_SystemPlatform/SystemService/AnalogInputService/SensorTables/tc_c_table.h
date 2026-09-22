#ifndef TC_C_TABLE_H
#define TC_C_TABLE_H

#include "PiecewiseLinearTable.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TC_C_TABLE_COMPLETE (1U)

#define TC_C_MIN_MILLICELSIUS   (0L)
#define TC_C_MAX_MILLICELSIUS   (2300000L)
#define TC_C_EXTENDED_MARGIN_MC (20000L)

#define TC_C_MEASUREMENT_SEGMENT_COUNT (117U)
#define TC_C_MEASUREMENT_BOUNDARY_COUNT (TC_C_MEASUREMENT_SEGMENT_COUNT + 1U)
#define TC_C_INPUT_SHIFT_UV (300L)
#define TC_C_MEASUREMENT_COEFFICIENT_SCALE (100UL)

#define TC_C_CJC_MIN_MILLICELSIUS (-20000L)
#define TC_C_CJC_MAX_MILLICELSIUS (109000L)
#define TC_C_CJC_INTERVAL_MC (10000L)
#define TC_C_CJC_SEGMENT_COUNT (13U)
#define TC_C_CJC_INPUT_SHIFT_DECICELSIUS (200L)
#define TC_C_CJC_COEFFICIENT_SCALE (1000UL)

const PiecewiseLinearTable_t *TcCTable_GetMeasurementTable(void);
const PiecewiseLinearTable_t *TcCTable_GetCjcTable(void);
bool TcCTable_IsReady(void);

#ifdef __cplusplus
}
#endif

#endif /* TC_C_TABLE_H */

