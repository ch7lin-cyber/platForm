#ifndef FB_ALARM_H
#define FB_ALARM_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FB_ALARM_OPTION_STANDBY (1U << 0)
#define FB_ALARM_OPTION_INVERT  (1U << 1)
#define FB_ALARM_OPTION_PEAK    (1U << 3)
#define FB_ALARM_OPTION_MASK    (0x000BU)
#define FB_ALARM_UNITS_PER_DEGREE_C (1000)
#define FB_ALARM_LIMIT_MIN      (-200000)
#define FB_ALARM_LIMIT_MAX      (3000000)
#define FB_ALARM_DELAY_MAX_MS   (100000U)
#define FB_ALARM_STANDBY_BAND_DEFAULT (100) /* +/-0.100 degree C */
#define FB_ALARM_STANDBY_BAND_MAX FB_ALARM_LIMIT_MAX

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

typedef enum
{
    FB_ALARM_RESET_AUTO = 0,
    FB_ALARM_RESET_MANUAL,
    FB_ALARM_RESET_MODE_COUNT
} FbAlarmResetMode_t;

typedef enum
{
    FB_ALARM_STATUS_OK = 0,
    FB_ALARM_STATUS_DISABLED,
    FB_ALARM_STATUS_INVALID_ARGUMENT,
    FB_ALARM_STATUS_INVALID_CONFIGURATION,
    FB_ALARM_STATUS_INVALID_INPUT,
    FB_ALARM_STATUS_RESET,
    FB_ALARM_STATUS_RESET_REJECTED
} FbAlarmStatus_t;

typedef struct
{
    FbAlarmMode_t mode;
    FbAlarmResetMode_t reset_mode; /* AUTO default; replaces HOLD option */
    int32_t alarm_high; /* temperature values use 0.001 degree C */
    int32_t alarm_low;  /* signed offsets in relative modes */
    int32_t standby_band; /* 0.001 degree C, 0..MAX; zero requires PV == SV */
    uint32_t delay_ms; /* assertion delay only, 0..100000 ms */
    uint16_t options;
} FbAlarmConfig_t;

typedef struct
{
    int32_t process_value; /* 0.001 degree C; temperature sensor snapshot uses same scale */
    int32_t setpoint;
    uint32_t elapsed_ms; /* actual interval since previous call */
    bool enable; /* false forces output OFF, regardless of inversion */
    bool valid;  /* PV/SV usable; false forces OFF and resets runtime */
    bool reset; /* rising edge clears latch only when condition is normal */
    bool clear_peak; /* rising edge invalidates historical peaks */
} FbAlarmInput_t;

typedef struct
{
    bool alarm_active; /* final logical output, after manual latch and inversion */
    bool condition_active; /* comparator result, gated by standby */
    bool qualified_active; /* condition after assertion delay, before manual latch */
    bool alarm_latched;
    bool standby_ready;
    bool delay_active;
    uint32_t delay_elapsed_ms;
    bool peak_valid; /* false until a qualified alarm sample is captured */
    int32_t peak_high; /* 0.001 degree C */
    int32_t peak_low;  /* 0.001 degree C */
    FbAlarmStatus_t status;
} FbAlarmOutput_t;

typedef struct
{
    FbAlarmConfig_t config;
    bool configured;
    bool alarm_latched;
    bool standby_ready;
    bool hysteresis_active;
    bool delay_active;
    uint32_t delay_elapsed_ms;
    bool peak_valid;
    int32_t peak_high;
    int32_t peak_low;
    bool reset_previous;
    bool clear_peak_previous;
} FbAlarm_t;

/* Initialize once before use; all-zero storage is also a valid initial state. */
void FbAlarm_Initialize(FbAlarm_t *instance);

/* Initializes configuration with mode OFF and the default standby band. */
void FbAlarm_DefaultConfig(FbAlarmConfig_t *config);

/*
 * One independent alarm per instance, no heap, clock or hardware access.
 * Pipeline: standby -> comparison/hysteresis -> assertion delay -> manual latch -> invert.
 * No event/mode 0 always returns OFF, bypassing inversion and recording.
 * At first observation the delay starts at zero; later elapsed_ms accumulate.
 * A false condition cancels timing. No separate OFF delay is applied.
 * MANUAL retains the alarm latch until RESET while condition_active is false.
 * RESET while condition_active is true is rejected without interrupting timing.
 * RESET only clears the latch, preserving standby, hysteresis and peak history.
 * Control STOP is not an input; it cannot clear or suppress manual latching.
 * Peak capture uses qualified samples only; a manual latch cannot extend capture.
 * Disable/invalid input clear runtime but retain peaks. Config changes clear both.
 * SV changes do not rearm standby or clear history. No separate OFF delay.
 * RESET/clear_peak are rising-edge pins, tracked even while disabled. When input
 * is NULL edge history is reset. State and arguments must not overlap; serialize
 * calls on an instance. See FbAlarm/README.md for project-specific policies.
 */
FbAlarmStatus_t FbAlarm_Execute(FbAlarm_t *instance,
                                const FbAlarmConfig_t *config,
                                const FbAlarmInput_t *input,
                                FbAlarmOutput_t *output);

#ifdef __cplusplus
}
#endif
#endif /* FB_ALARM_H */
