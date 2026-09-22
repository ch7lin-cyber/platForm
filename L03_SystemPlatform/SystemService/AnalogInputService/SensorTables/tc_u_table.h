#ifndef TC_U_TABLE_H
#define TC_U_TABLE_H

#include "PiecewiseLinearTable.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TC_U_TABLE_COMPLETE (1U)

#define TC_U_MIN_MILLICELSIUS   (-200000L)
#define TC_U_MAX_MILLICELSIUS   (500000L)
#define TC_U_EXTENDED_MARGIN_MC (20000L)

#define TC_U_MEASUREMENT_SEGMENT_COUNT (40U)
#define TC_U_MEASUREMENT_BOUNDARY_COUNT (TC_U_MEASUREMENT_SEGMENT_COUNT + 1U)
#define TC_U_INPUT_SHIFT_UV (5800L)
#define TC_U_MEASUREMENT_COEFFICIENT_SCALE (100UL)

#define TC_U_CJC_MIN_MILLICELSIUS (-20000L)
#define TC_U_CJC_MAX_MILLICELSIUS (109000L)
#define TC_U_CJC_INTERVAL_MC (10000L)
#define TC_U_CJC_SEGMENT_COUNT (13U)
#define TC_U_CJC_INPUT_SHIFT_DECICELSIUS (200L)
#define TC_U_CJC_COEFFICIENT_SCALE (1000UL)

const PiecewiseLinearTable_t *TcUTable_GetMeasurementTable(void);
const PiecewiseLinearTable_t *TcUTable_GetCjcTable(void);
bool TcUTable_IsReady(void);

#ifdef __cplusplus
}
#endif

#endif /* TC_U_TABLE_H */

