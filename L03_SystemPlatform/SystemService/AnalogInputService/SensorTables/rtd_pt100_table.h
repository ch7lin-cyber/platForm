#ifndef RTD_PT100_TABLE_H
#define RTD_PT100_TABLE_H

#include "PiecewiseLinearTable.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RTD_PT100_TABLE_COMPLETE (1U)

#define RTD_PT100_MIN_MILLICELSIUS   (-200000L)
#define RTD_PT100_MAX_MILLICELSIUS   (850000L)
#define RTD_PT100_EXTENDED_MARGIN_MC (20000L)

#define RTD_PT100_MEASUREMENT_SEGMENT_COUNT (55U)
#define RTD_PT100_MEASUREMENT_BOUNDARY_COUNT (RTD_PT100_MEASUREMENT_SEGMENT_COUNT + 1U)
#define RTD_PT100_INPUT_SHIFT_MILLIOHM (-9800L)
#define RTD_PT100_MEASUREMENT_COEFFICIENT_SCALE (100UL)

const PiecewiseLinearTable_t *RtdPt100Table_GetMeasurementTable(void);
bool RtdPt100Table_IsReady(void);

#ifdef __cplusplus
}
#endif

#endif /* RTD_PT100_TABLE_H */

