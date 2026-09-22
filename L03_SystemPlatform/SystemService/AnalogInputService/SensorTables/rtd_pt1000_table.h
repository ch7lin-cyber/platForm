#ifndef RTD_PT1000_TABLE_H
#define RTD_PT1000_TABLE_H

#include "PiecewiseLinearTable.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RTD_PT1000_TABLE_COMPLETE (1U)

#define RTD_PT1000_MIN_MILLICELSIUS   (-200000L)
#define RTD_PT1000_MAX_MILLICELSIUS   (850000L)
#define RTD_PT1000_EXTENDED_MARGIN_MC (20000L)

#define RTD_PT1000_MEASUREMENT_SEGMENT_COUNT (55U)
#define RTD_PT1000_MEASUREMENT_BOUNDARY_COUNT (RTD_PT1000_MEASUREMENT_SEGMENT_COUNT + 1U)
#define RTD_PT1000_INPUT_SHIFT_MILLIOHM (-98900L)
#define RTD_PT1000_MEASUREMENT_COEFFICIENT_SCALE (100UL)

const PiecewiseLinearTable_t *RtdPt1000Table_GetMeasurementTable(void);
bool RtdPt1000Table_IsReady(void);

#ifdef __cplusplus
}
#endif

#endif /* RTD_PT1000_TABLE_H */

