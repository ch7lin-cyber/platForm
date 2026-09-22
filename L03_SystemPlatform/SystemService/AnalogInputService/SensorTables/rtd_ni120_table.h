#ifndef RTD_NI120_TABLE_H
#define RTD_NI120_TABLE_H

#include "PiecewiseLinearTable.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RTD_NI120_TABLE_COMPLETE (1U)

#define RTD_NI120_MIN_MILLICELSIUS   (-80000L)
#define RTD_NI120_MAX_MILLICELSIUS   (300000L)
#define RTD_NI120_EXTENDED_MARGIN_MC (20000L)

#define RTD_NI120_MEASUREMENT_SEGMENT_COUNT (21U)
#define RTD_NI120_MEASUREMENT_BOUNDARY_COUNT (RTD_NI120_MEASUREMENT_SEGMENT_COUNT + 1U)
#define RTD_NI120_INPUT_SHIFT_MILLIOHM (-53500L)
#define RTD_NI120_MEASUREMENT_COEFFICIENT_SCALE (100UL)

const PiecewiseLinearTable_t *RtdNi120Table_GetMeasurementTable(void);
bool RtdNi120Table_IsReady(void);

#ifdef __cplusplus
}
#endif

#endif /* RTD_NI120_TABLE_H */

