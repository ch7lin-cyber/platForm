#ifndef TC_E_TABLE_H
#define TC_E_TABLE_H

#include "PiecewiseLinearTable.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TC_E_TABLE_COMPLETE (1U)

#define TC_E_MIN_MILLICELSIUS   (0L)
#define TC_E_MAX_MILLICELSIUS   (600000L)
#define TC_E_EXTENDED_MARGIN_MC (20000L)

#define TC_E_MEASUREMENT_SEGMENT_COUNT (33U)
#define TC_E_MEASUREMENT_BOUNDARY_COUNT (TC_E_MEASUREMENT_SEGMENT_COUNT + 1U)
#define TC_E_INPUT_SHIFT_UV (2300L)
#define TC_E_MEASUREMENT_COEFFICIENT_SCALE (100UL)

#define TC_E_CJC_MIN_MILLICELSIUS (-20000L)
#define TC_E_CJC_MAX_MILLICELSIUS (109000L)
#define TC_E_CJC_INTERVAL_MC (10000L)
#define TC_E_CJC_SEGMENT_COUNT (13U)
#define TC_E_CJC_INPUT_SHIFT_DECICELSIUS (200L)
#define TC_E_CJC_COEFFICIENT_SCALE (1000UL)

const PiecewiseLinearTable_t *TcETable_GetMeasurementTable(void);
const PiecewiseLinearTable_t *TcETable_GetCjcTable(void);
bool TcETable_IsReady(void);

#ifdef __cplusplus
}
#endif

#endif /* TC_E_TABLE_H */

