#include "SensorConversionService.h"

#include <limits.h>
#include <stddef.h>
#include <string.h>

#define ANALOG_OUTPUT_MIN (-1999L)
#define ANALOG_OUTPUT_MAX (19999L)

bool SensorConversionService_IsThermocouple(SensorConversionType_t type)
{
    return type <= SENSOR_TYPE_TC_D;
}

bool SensorConversionService_IsRtd(SensorConversionType_t type)
{
    return (type >= SENSOR_TYPE_RTD_PT100) &&
           (type <= SENSOR_TYPE_RTD_PT1000);
}

static bool StoreInt32(int64_t value, int32_t *output)
{
    if ((output == NULL) || (value < INT32_MIN) || (value > INT32_MAX))
    {
        return false;
    }
    *output = (int32_t)value;
    return true;
}

static void SetLimitFlags(const SensorConversionConfig_t *config,
                          SensorConversionResult_t *result)
{
    int64_t extended_min = (int64_t)config->nominal_min -
                           (int64_t)config->extended_margin;
    int64_t extended_max = (int64_t)config->nominal_max +
                           (int64_t)config->extended_margin;

    if ((result->engineering_value < config->nominal_min) ||
        (result->engineering_value > config->nominal_max))
    {
        result->flags |= SENSOR_FLAG_OUT_OF_SPEC;
    }
    if (((int64_t)result->engineering_value < extended_min) ||
        ((int64_t)result->engineering_value > extended_max))
    {
        result->flags |= SENSOR_FLAG_OUT_OF_RANGE;
    }
}

static bool ScaleAnalog(int32_t signal, int32_t signal_min,
                        int32_t signal_max, int32_t *value)
{
    int64_t numerator;

    if ((signal_max <= signal_min) || (value == NULL))
    {
        return false;
    }
    numerator = ((int64_t)signal - signal_min) *
                (ANALOG_OUTPUT_MAX - ANALOG_OUTPUT_MIN);
    return StoreInt32(ANALOG_OUTPUT_MIN +
                      (numerator / (signal_max - signal_min)), value);
}

bool SensorConversionService_Convert(
    const SensorConversionConfig_t *config,
    int32_t calibrated_uv,
    int32_t cold_junction_millicelsius,
    SensorConversionResult_t *result)
{
    int32_t table_input;
    int64_t physical;

    if ((config == NULL) || (result == NULL))
    {
        return false;
    }
    (void)memset(result, 0, sizeof(*result));
    result->calibrated_uv = calibrated_uv;

    if (SensorConversionService_IsThermocouple(config->type))
    {
        if (!PiecewiseLinearTable_Evaluate(config->cjc_table,
                                           cold_junction_millicelsius,
                                           &result->cjc_uv))
        {
            result->flags |= SENSOR_FLAG_CJC_ERROR;
            return false;
        }
        if (!StoreInt32((int64_t)calibrated_uv + result->cjc_uv,
                        &table_input) ||
            !PiecewiseLinearTable_Evaluate(config->measurement_table,
                                           table_input,
                                           &result->engineering_value))
        {
            result->flags |= SENSOR_FLAG_TABLE_ERROR;
            return false;
        }
        result->physical_input = table_input;
    }
    else if (SensorConversionService_IsRtd(config->type))
    {
        if ((config->excitation_current_ua == 0UL) ||
            !StoreInt32(((int64_t)calibrated_uv * 1000LL) /
                        config->excitation_current_ua,
                        &result->physical_input) ||
            !PiecewiseLinearTable_Evaluate(config->measurement_table,
                                           result->physical_input,
                                           &result->engineering_value))
        {
            result->flags |= SENSOR_FLAG_TABLE_ERROR;
            return false;
        }
    }
    else
    {
        if ((config->frontend_ratio_denominator == 0UL) ||
            !StoreInt32(((int64_t)calibrated_uv *
                         config->frontend_ratio_numerator) /
                        config->frontend_ratio_denominator,
                        &result->physical_input))
        {
            return false;
        }
        physical = result->physical_input;
        if ((config->type == SENSOR_TYPE_CURRENT_0_20MA) ||
            (config->type == SENSOR_TYPE_CURRENT_4_20MA))
        {
            if (config->shunt_resistance_milliohm == 0UL)
            {
                return false;
            }
            physical = (physical * 1000LL) /
                       config->shunt_resistance_milliohm;
            if (!StoreInt32(physical, &result->physical_input))
            {
                return false;
            }
            if (!ScaleAnalog(result->physical_input,
                             (config->type == SENSOR_TYPE_CURRENT_4_20MA) ?
                                 4000L : 0L,
                             20000L, &result->engineering_value))
            {
                return false;
            }
        }
        else
        {
            int32_t full_scale =
                (config->type == SENSOR_TYPE_VOLTAGE_0_5V) ? 5000000L :
                (config->type == SENSOR_TYPE_VOLTAGE_0_10V) ? 10000000L :
                50000L;
            if (!ScaleAnalog(result->physical_input, 0L, full_scale,
                             &result->engineering_value))
            {
                return false;
            }
        }
    }
    SetLimitFlags(config, result);
    return true;
}
