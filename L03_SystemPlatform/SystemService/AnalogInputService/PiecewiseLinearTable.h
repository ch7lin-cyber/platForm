#ifndef PIECEWISE_LINEAR_TABLE_H
#define PIECEWISE_LINEAR_TABLE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    int32_t x_min;
    int32_t x_max;
    int32_t slope;
    int64_t intercept;
} PiecewiseLinearSegment_t;

typedef struct
{
    const PiecewiseLinearSegment_t *segments;
    uint16_t segment_count;
    uint32_t coefficient_scale;
} PiecewiseLinearTable_t;

bool PiecewiseLinearTable_Evaluate(const PiecewiseLinearTable_t *table,
                                   int32_t x,
                                   int32_t *y);
bool PiecewiseLinearTable_IsValid(const PiecewiseLinearTable_t *table);

#endif
