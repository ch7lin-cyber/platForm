#ifndef TC_K_TABLE_H
#define TC_K_TABLE_H

#include "PiecewiseLinearTable.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TC_K_TABLE_COMPLETE (1U)

#define TC_K_MIN_MILLICELSIUS   (-200000L)
#define TC_K_MAX_MILLICELSIUS   (1300000L)
#define TC_K_EXTENDED_MARGIN_MC (20000L)

#define TC_K_MEASUREMENT_SEGMENT_COUNT (77U)
#define TC_K_MEASUREMENT_BOUNDARY_COUNT (TC_K_MEASUREMENT_SEGMENT_COUNT + 1U)
#define TC_K_INPUT_SHIFT_UV (6200L)
#define TC_K_MEASUREMENT_COEFFICIENT_SCALE (100UL)

#define TC_K_CJC_MIN_MILLICELSIUS (-20000L)
#define TC_K_CJC_MAX_MILLICELSIUS (109000L)
#define TC_K_CJC_INTERVAL_MC (10000L)
#define TC_K_CJC_SEGMENT_COUNT (13U)
#define TC_K_CJC_INPUT_SHIFT_DECICELSIUS (200L)
#define TC_K_CJC_COEFFICIENT_SCALE (1000UL)

const PiecewiseLinearTable_t *TcKTable_GetMeasurementTable(void);
const PiecewiseLinearTable_t *TcKTable_GetCjcTable(void);
bool TcKTable_IsReady(void);

#ifdef __cplusplus
}
#endif

#endif /* TC_K_TABLE_H */

