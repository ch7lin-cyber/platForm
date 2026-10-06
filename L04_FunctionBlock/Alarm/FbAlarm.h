#ifndef FB_ALARM_H
#define FB_ALARM_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FB_ALARM_CLEAR_LATCH       (1U << 0)
#define FB_ALARM_CLEAR_PEAK        (1U << 1)

#define FB_ALARM_OPTION_STANDBY    (1U << 0)
#define FB_ALARM_OPTION_INVERT     (1U << 1)
#define FB_ALARM_OPTION_HOLD       (1U << 2)
#define FB_ALARM_OPTION_PEAK       (1U << 3)

#define FB_ALARM_STANDBY_BAND      (10)

typedef enum
{
    FB_ALARM_MODE_DISABLED = 0,
    FB_ALARM_MODE_RELATIVE_HIGH_LOW,
    FB_ALARM_MODE_RELATIVE_HIGH,
    FB_ALARM_MODE_RELATIVE_LOW,
    FB_ALARM_MODE_ABSOLUTE_HIGH_LOW,
    FB_ALARM_MODE_ABSOLUTE_HIGH,
    FB_ALARM_MODE_ABSOLUTE_LOW,
    FB_ALARM_MODE_HYSTERESIS_HIGH,
    FB_ALARM_MODE_HYSTERESIS_LOW,
    FB_ALARM_MODE_COUNT
} FbAlarmMode_t;

typedef struct
{
    bool hold_latched;
    bool standby_ready;
    bool hysteresis_active;
    bool peak_initialized;
    int16_t peak_high;
    int16_t peak_low;
} FbAlarm_t;

typedef struct
{
    bool alarm_active;
    int16_t peak_high;
    int16_t peak_low;
} FbAlarmOutput_t;

void FbAlarm_Initialize(FbAlarm_t *instance,
                        int16_t initial_process_value);

/*
 * Executes one cyclic alarm evaluation.
 *
 * Relative and hysteresis thresholds must be non-negative. Absolute limits
 * may be signed. Intermediate comparisons use 32-bit arithmetic so int16_t
 * setpoint/offset combinations cannot overflow.
 */
bool FbAlarm_Execute(FbAlarm_t *instance,
                     int16_t process_value,
                     int16_t setpoint,
                     FbAlarmMode_t mode,
                     int16_t alarm_high,
                     int16_t alarm_low,
                     uint16_t clear_request,
                     uint16_t options,
                     FbAlarmOutput_t *output);

#ifdef __cplusplus
}
#endif

#endif /* FB_ALARM_H */
