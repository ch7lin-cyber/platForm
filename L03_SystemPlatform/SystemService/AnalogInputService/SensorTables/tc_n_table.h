#ifndef TC_N_TABLE_H
#define TC_N_TABLE_H

#include "PiecewiseLinearTable.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TC_N_TABLE_COMPLETE (1U)

#define TC_N_MIN_MILLICELSIUS   (-200000L)
#define TC_N_MAX_MILLICELSIUS   (1300000L)
#define TC_N_EXTENDED_MARGIN_MC (20000L)

#define TC_N_MEASUREMENT_SEGMENT_COUNT (76U)
#define TC_N_MEASUREMENT_BOUNDARY_COUNT (TC_N_MEASUREMENT_SEGMENT_COUNT + 1U)
#define TC_N_INPUT_SHIFT_UV (4200L)
#define TC_N_MEASUREMENT_COEFFICIENT_SCALE (100UL)

#define TC_N_CJC_MIN_MILLICELSIUS (-20000L)
#define TC_N_CJC_MAX_MILLICELSIUS (109000L)
#define TC_N_CJC_INTERVAL_MC (10000L)
#define TC_N_CJC_SEGMENT_COUNT (13U)
#define TC_N_CJC_INPUT_SHIFT_DECICELSIUS (200L)
#define TC_N_CJC_COEFFICIENT_SCALE (1000UL)

const PiecewiseLinearTable_t *TcNTable_GetMeasurementTable(void);
const PiecewiseLinearTable_t *TcNTable_GetCjcTable(void);
bool TcNTable_IsReady(void);

#ifdef __cplusplus
}
#endif

#endif /* TC_N_TABLE_H */

