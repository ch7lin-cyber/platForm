#ifndef RTD_JPT100_TABLE_H
#define RTD_JPT100_TABLE_H

#include "PiecewiseLinearTable.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RTD_JPT100_TABLE_COMPLETE (1U)

#define RTD_JPT100_MIN_MILLICELSIUS   (-20000L)
#define RTD_JPT100_MAX_MILLICELSIUS   (400000L)
#define RTD_JPT100_EXTENDED_MARGIN_MC (20000L)

#define RTD_JPT100_MEASUREMENT_SEGMENT_COUNT (40U)
#define RTD_JPT100_MEASUREMENT_BOUNDARY_COUNT (RTD_JPT100_MEASUREMENT_SEGMENT_COUNT + 1U)
#define RTD_JPT100_INPUT_SHIFT_MILLIOHM (-16500L)
#define RTD_JPT100_MEASUREMENT_COEFFICIENT_SCALE (100UL)

const PiecewiseLinearTable_t *RtdJpt100Table_GetMeasurementTable(void);
bool RtdJpt100Table_IsReady(void);

#ifdef __cplusplus
}
#endif

#endif /* RTD_JPT100_TABLE_H */

