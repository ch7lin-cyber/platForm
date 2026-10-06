#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "FbAlarm.h"

static bool Execute(FbAlarm_t *alarm,
                    int16_t process_value,
                    int16_t setpoint,
                    FbAlarmMode_t mode,
                    int16_t alarm_high,
                    int16_t alarm_low,
                    uint16_t clear_request,
                    uint16_t options,
                    FbAlarmOutput_t *output)
{
    return FbAlarm_Execute(alarm, process_value, setpoint, mode,
                           alarm_high, alarm_low, clear_request,
                           options, output);
}

static void TestRelativeAndAbsoluteModes(void)
{
    FbAlarm_t alarm;
    FbAlarmOutput_t output;

    FbAlarm_Initialize(&alarm, 100);
    assert(Execute(&alarm, 111, 100, FB_ALARM_MODE_RELATIVE_HIGH,
                   10, 10, 0U, 0U, &output));
    assert(output.alarm_active);
    assert(Execute(&alarm, 89, 100, FB_ALARM_MODE_RELATIVE_LOW,
                   10, 10, 0U, 0U, &output));
    assert(output.alarm_active);
    assert(Execute(&alarm, 100, 100, FB_ALARM_MODE_RELATIVE_HIGH_LOW,
                   10, 10, 0U, 0U, &output));
    assert(!output.alarm_active);

    assert(Execute(&alarm, 151, 0, FB_ALARM_MODE_ABSOLUTE_HIGH,
                   150, -50, 0U, 0U, &output));
    assert(output.alarm_active);
    assert(Execute(&alarm, -51, 0, FB_ALARM_MODE_ABSOLUTE_LOW,
                   150, -50, 0U, 0U, &output));
    assert(output.alarm_active);
    assert(Execute(&alarm, 0, 0, FB_ALARM_MODE_ABSOLUTE_HIGH_LOW,
                   150, -50, 0U, 0U, &output));
    assert(!output.alarm_active);
}

static void TestHighAndLowHysteresis(void)
{
    FbAlarm_t alarm;
    FbAlarmOutput_t output;

    FbAlarm_Initialize(&alarm, 100);
    assert(Execute(&alarm, 111, 100, FB_ALARM_MODE_HYSTERESIS_HIGH,
                   10, 5, 0U, 0U, &output));
    assert(output.alarm_active);
    assert(Execute(&alarm, 107, 100, FB_ALARM_MODE_HYSTERESIS_HIGH,
                   10, 5, 0U, 0U, &output));
    assert(output.alarm_active);
    assert(Execute(&alarm, 104, 100, FB_ALARM_MODE_HYSTERESIS_HIGH,
                   10, 5, 0U, 0U, &output));
    assert(!output.alarm_active);

    assert(Execute(&alarm, 89, 100, FB_ALARM_MODE_HYSTERESIS_LOW,
                   10, 5, 0U, 0U, &output));
    assert(output.alarm_active);
    assert(Execute(&alarm, 93, 100, FB_ALARM_MODE_HYSTERESIS_LOW,
                   10, 5, 0U, 0U, &output));
    assert(output.alarm_active);
    assert(Execute(&alarm, 96, 100, FB_ALARM_MODE_HYSTERESIS_LOW,
                   10, 5, 0U, 0U, &output));
    assert(!output.alarm_active);
}

static void TestStandbyHoldInvertAndClear(void)
{
    FbAlarm_t alarm;
    FbAlarmOutput_t output;
    uint16_t standby_hold = FB_ALARM_OPTION_STANDBY |
                            FB_ALARM_OPTION_HOLD;

    FbAlarm_Initialize(&alarm, 0);
    assert(Execute(&alarm, 130, 100, FB_ALARM_MODE_RELATIVE_HIGH,
                   10, 10, 0U, standby_hold, &output));
    assert(!output.alarm_active);
    assert(!alarm.standby_ready);

    assert(Execute(&alarm, 105, 100, FB_ALARM_MODE_RELATIVE_HIGH,
                   10, 10, 0U, standby_hold, &output));
    assert(alarm.standby_ready);
    assert(!output.alarm_active);
    assert(Execute(&alarm, 120, 100, FB_ALARM_MODE_RELATIVE_HIGH,
                   10, 10, 0U, standby_hold, &output));
    assert(output.alarm_active);
    assert(Execute(&alarm, 100, 100, FB_ALARM_MODE_RELATIVE_HIGH,
                   10, 10, 0U, standby_hold, &output));
    assert(output.alarm_active);
    assert(Execute(&alarm, 100, 100, FB_ALARM_MODE_RELATIVE_HIGH,
                   10, 10, FB_ALARM_CLEAR_LATCH, standby_hold, &output));
    assert(!output.alarm_active);

    assert(Execute(&alarm, 100, 100, FB_ALARM_MODE_DISABLED,
                   0, 0, 0U, FB_ALARM_OPTION_INVERT, &output));
    assert(output.alarm_active);
}

static void TestPeakTrackingAndClear(void)
{
    FbAlarm_t alarm;
    FbAlarmOutput_t output;

    FbAlarm_Initialize(&alarm, 100);
    assert(Execute(&alarm, 120, 100, FB_ALARM_MODE_DISABLED,
                   0, 0, 0U, FB_ALARM_OPTION_PEAK, &output));
    assert(output.peak_high == 120);
    assert(output.peak_low == 100);
    assert(Execute(&alarm, 80, 100, FB_ALARM_MODE_DISABLED,
                   0, 0, 0U, FB_ALARM_OPTION_PEAK, &output));
    assert(output.peak_high == 120);
    assert(output.peak_low == 80);
    assert(Execute(&alarm, 95, 100, FB_ALARM_MODE_DISABLED,
                   0, 0, FB_ALARM_CLEAR_PEAK,
                   FB_ALARM_OPTION_PEAK, &output));
    assert(output.peak_high == 95);
    assert(output.peak_low == 95);
}

static void TestValidationAndOverflowSafeComparisons(void)
{
    FbAlarm_t alarm;
    FbAlarmOutput_t output;

    FbAlarm_Initialize(&alarm, 0);
    assert(!Execute(NULL, 0, 0, FB_ALARM_MODE_DISABLED,
                    0, 0, 0U, 0U, &output));
    assert(!Execute(&alarm, 0, 0, FB_ALARM_MODE_DISABLED,
                    0, 0, 0U, 0U, NULL));
    assert(!Execute(&alarm, 0, 0, (FbAlarmMode_t)99,
                    0, 0, 0U, 0U, &output));
    assert(!Execute(&alarm, 0, 0, FB_ALARM_MODE_RELATIVE_HIGH,
                    -1, 0, 0U, 0U, &output));
    assert(!Execute(&alarm, 0, 0, FB_ALARM_MODE_ABSOLUTE_HIGH_LOW,
                    -10, 10, 0U, 0U, &output));

    assert(Execute(&alarm, INT16_MAX, INT16_MAX,
                   FB_ALARM_MODE_RELATIVE_HIGH, 100, 0,
                   0U, 0U, &output));
    assert(!output.alarm_active);
    assert(Execute(&alarm, INT16_MIN, INT16_MIN,
                   FB_ALARM_MODE_RELATIVE_LOW, 0, 100,
                   0U, 0U, &output));
    assert(!output.alarm_active);
}

int main(void)
{
    TestRelativeAndAbsoluteModes();
    TestHighAndLowHysteresis();
    TestStandbyHoldInvertAndClear();
    TestPeakTrackingAndClear();
    TestValidationAndOverflowSafeComparisons();
    return 0;
}
