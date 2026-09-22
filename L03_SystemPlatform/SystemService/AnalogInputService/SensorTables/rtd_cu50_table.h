#ifndef RTD_CU50_TABLE_H
#define RTD_CU50_TABLE_H

#include "PiecewiseLinearTable.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RTD_CU50_TABLE_COMPLETE (1U)

#define RTD_CU50_MIN_MILLICELSIUS   (-50000L)
#define RTD_CU50_MAX_MILLICELSIUS   (150000L)
#define RTD_CU50_EXTENDED_MARGIN_MC (20000L)

#define RTD_CU50_MEASUREMENT_SEGMENT_COUNT (12U)
#define RTD_CU50_MEASUREMENT_BOUNDARY_COUNT (RTD_CU50_MEASUREMENT_SEGMENT_COUNT + 1U)
#define RTD_CU50_INPUT_SHIFT_MILLIOHM (-35000L)
#define RTD_CU50_MEASUREMENT_COEFFICIENT_SCALE (100UL)

const PiecewiseLinearTable_t *RtdCu50Table_GetMeasurementTable(void);
bool RtdCu50Table_IsReady(void);

#ifdef __cplusplus
}
#endif

#endif /* RTD_CU50_TABLE_H */

