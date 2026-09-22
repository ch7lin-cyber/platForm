#ifndef TC_R_TABLE_H
#define TC_R_TABLE_H

#include "PiecewiseLinearTable.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TC_R_TABLE_COMPLETE (1U)

#define TC_R_MIN_MILLICELSIUS   (0L)
#define TC_R_MAX_MILLICELSIUS   (1700000L)
#define TC_R_EXTENDED_MARGIN_MC (20000L)

#define TC_R_MEASUREMENT_SEGMENT_COUNT (87U)
#define TC_R_MEASUREMENT_BOUNDARY_COUNT (TC_R_MEASUREMENT_SEGMENT_COUNT + 1U)
#define TC_R_INPUT_SHIFT_UV (110L)
#define TC_R_MEASUREMENT_COEFFICIENT_SCALE (100UL)

#define TC_R_CJC_MIN_MILLICELSIUS (-20000L)
#define TC_R_CJC_MAX_MILLICELSIUS (109000L)
#define TC_R_CJC_INTERVAL_MC (10000L)
#define TC_R_CJC_SEGMENT_COUNT (13U)
#define TC_R_CJC_INPUT_SHIFT_DECICELSIUS (200L)
#define TC_R_CJC_COEFFICIENT_SCALE (1000UL)

const PiecewiseLinearTable_t *TcRTable_GetMeasurementTable(void);
const PiecewiseLinearTable_t *TcRTable_GetCjcTable(void);
bool TcRTable_IsReady(void);

#ifdef __cplusplus
}
#endif

#endif /* TC_R_TABLE_H */

