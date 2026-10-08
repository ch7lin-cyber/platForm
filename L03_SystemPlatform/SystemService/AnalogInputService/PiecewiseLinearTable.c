#include "PiecewiseLinearTable.h"

#include <stddef.h>

/* Keep the comparison signed regardless of the target's INT32_MIN macro. */
#define TABLE_INT32_MIN_I64 (-2147483647LL - 1LL)
#define TABLE_INT32_MAX_I64 (2147483647LL)

bool PiecewiseLinearTable_IsValid(const PiecewiseLinearTable_t *table)
{
    uint16_t index;

    if ((table == NULL) || (table->segments == NULL) ||
        (table->segment_count == 0U) || (table->coefficient_scale == 0UL))
    {
        return false;
    }
    for (index = 0U; index < table->segment_count; index++)
    {
        if (table->segments[index].x_min > table->segments[index].x_max)
        {
            return false;
        }
        if ((index > 0U) &&
            (table->segments[index].x_min <=
             table->segments[index - 1U].x_min))
        {
            return false;
        }
    }
    return true;
}

bool PiecewiseLinearTable_Evaluate(const PiecewiseLinearTable_t *table,
                                   int32_t x,
                                   int32_t *y)
{
    uint16_t low;
    uint16_t high;
    int64_t value;

    if (!PiecewiseLinearTable_IsValid(table) || (y == NULL))
    {
        return false;
    }
    low = 0U;
    high = table->segment_count;
    while (low < high)
    {
        uint16_t middle = (uint16_t)(low + ((high - low) / 2U));
        const PiecewiseLinearSegment_t *segment = &table->segments[middle];
        if (x < segment->x_min)
        {
            high = middle;
        }
        else if (x > segment->x_max)
        {
            low = (uint16_t)(middle + 1U);
        }
        else
        {
            value = ((int64_t)segment->slope * (int64_t)x) +
                    segment->intercept;
            value /= (int64_t)table->coefficient_scale;
            if ((value < TABLE_INT32_MIN_I64) ||
                (value > TABLE_INT32_MAX_I64))
            {
                return false;
            }
            *y = (int32_t)value;
            return true;
        }
    }
    return false;
}
