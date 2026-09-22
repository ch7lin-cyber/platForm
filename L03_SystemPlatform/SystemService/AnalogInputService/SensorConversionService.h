#ifndef SENSOR_CONVERSION_SERVICE_H
#define SENSOR_CONVERSION_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

#include "PiecewiseLinearTable.h"

typedef enum
{
    SENSOR_TYPE_TC_K = 0,
    SENSOR_TYPE_TC_J,
    SENSOR_TYPE_TC_T,
    SENSOR_TYPE_TC_E,
    SENSOR_TYPE_TC_N,
    SENSOR_TYPE_TC_R,
    SENSOR_TYPE_TC_S,
    SENSOR_TYPE_TC_B,
    SENSOR_TYPE_TC_L,
    SENSOR_TYPE_TC_U,
    SENSOR_TYPE_TC_TXK,
    SENSOR_TYPE_TC_C,
    SENSOR_TYPE_TC_D,
    SENSOR_TYPE_RTD_PT100,
    SENSOR_TYPE_RTD_JPT100,
    SENSOR_TYPE_RTD_NI120,
    SENSOR_TYPE_RTD_CU50,
    SENSOR_TYPE_RTD_PT1000,
    SENSOR_TYPE_VOLTAGE_0_5V,
    SENSOR_TYPE_VOLTAGE_0_10V,
    SENSOR_TYPE_VOLTAGE_0_50MV,
    SENSOR_TYPE_CURRENT_0_20MA,
    SENSOR_TYPE_CURRENT_4_20MA
} SensorConversionType_t;

#define SENSOR_FLAG_OUT_OF_SPEC  (1UL << 0)
#define SENSOR_FLAG_OUT_OF_RANGE (1UL << 1)
#define SENSOR_FLAG_TABLE_ERROR  (1UL << 2)
#define SENSOR_FLAG_CJC_ERROR    (1UL << 3)

typedef struct
{
    SensorConversionType_t type;
    int32_t nominal_min;
    int32_t nominal_max;
    int32_t extended_margin;
    const PiecewiseLinearTable_t *measurement_table;
    const PiecewiseLinearTable_t *cjc_table;
    uint32_t excitation_current_ua;
    uint32_t shunt_resistance_milliohm;
    uint32_t frontend_ratio_numerator;
    uint32_t frontend_ratio_denominator;
} SensorConversionConfig_t;

typedef struct
{
    int32_t calibrated_uv;
    int32_t cjc_uv;
    int32_t physical_input;
    int32_t engineering_value;
    uint32_t flags;
} SensorConversionResult_t;

bool SensorConversionService_IsThermocouple(SensorConversionType_t type);
bool SensorConversionService_IsRtd(SensorConversionType_t type);
bool SensorConversionService_Convert(
    const SensorConversionConfig_t *config,
    int32_t calibrated_uv,
    int32_t cold_junction_millicelsius,
    SensorConversionResult_t *result);

#endif
