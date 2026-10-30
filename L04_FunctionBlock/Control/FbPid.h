#ifndef FB_PID_H
#define FB_PID_H

#include <stdbool.h>
#include <stdint.h>

/* Explicit build-wide opt-in. Fixed MV wiring test, NOT a PID algorithm. */
#ifndef FB_PID_TEST_BACKEND
#define FB_PID_TEST_BACKEND 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    FB_PID_MODE_AUTO_TUNE = 0,
    FB_PID_MODE_ON_OFF,
    FB_PID_MODE_PID,
    FB_PID_MODE_PID_FB,
    FB_PID_MODE_COUNT
} FbPidMode_t;

typedef enum
{
    FB_PID_ACTION_HEAT = 0,
    FB_PID_ACTION_COOL,
    FB_PID_ACTION_HEAT_COOL
} FbPidControlAction_t;

typedef enum
{
    FB_PID_PARAMETER_ON_OFF_OUT1_TS = 0,
    FB_PID_PARAMETER_ON_OFF_OUT2_TS,
    FB_PID_PARAMETER_DEADBAND,
    FB_PID_PARAMETER_PB,
    FB_PID_PARAMETER_TI,
    FB_PID_PARAMETER_TD,
    FB_PID_PARAMETER_COOL_PB,
    FB_PID_PARAMETER_COOL_TI,
    FB_PID_PARAMETER_COOL_TD,
    FB_PID_PARAMETER_MV1_LIMIT_HIGH,
    FB_PID_PARAMETER_MV1_LIMIT_LOW,
    FB_PID_PARAMETER_MV2_LIMIT_HIGH,
    FB_PID_PARAMETER_MV2_LIMIT_LOW,
    FB_PID_PARAMETER_ALPHA,
    FB_PID_PARAMETER_BETA,
    FB_PID_PARAMETER_GAMMA,
    FB_PID_PARAMETER_IPRELOAD,
    FB_PID_PARAMETER_PDOF,
    FB_PID_PARAMETER_PID_MODE,
    FB_PID_PARAMETER_DISTURBANCE_COMPENSATION,
    FB_PID_PARAMETER_AUTOTUNE_SV_RATIO,
    FB_PID_PARAMETER_AUTOTUNE_PB_RATIO,
    FB_PID_PARAMETER_AUTOTUNE_TI_RATIO,
    FB_PID_PARAMETER_AUTOTUNE_MV1_LIMIT_HIGH,
    FB_PID_PARAMETER_AUTOTUNE_MV2_LIMIT_HIGH,
    FB_PID_PARAMETER_AUTOTUNE_DISTURBANCE_COMPENSATION
} FbPidParameter_t;

typedef enum
{
    FB_PID_STATUS_OK = 0,
    FB_PID_STATUS_DISABLED,
    FB_PID_STATUS_RESET,
    FB_PID_STATUS_INVALID_ARGUMENT,
    FB_PID_STATUS_INVALID_CONFIG,
    FB_PID_STATUS_INVALID_INPUT,
    FB_PID_STATUS_INVALID_TIMING,
    FB_PID_STATUS_BACKEND_ERROR,
    FB_PID_STATUS_NOT_INITIALIZED
} FbPidStatus_t;

typedef struct
{
    FbPidMode_t mode;
    FbPidMode_t auto_tune_target_mode;
    FbPidControlAction_t control_action;
    uint32_t auto_tune_period_ms;
    uint32_t auto_tune_period_tolerance_ms;
} FbPidConfig_t;

typedef struct
{
    int32_t pv; /* public temperature unit: 0.001 degree C */
    int32_t sv; /* public temperature unit: 0.001 degree C */
    uint32_t elapsed_ms;
    bool enable;
    bool reset; /* rising-edge reset command */
    bool valid;
} FbPidInput_t;

typedef struct
{
    int32_t mv[2]; /* 0.1%: 0..1000; heat/cool may return two values */
    uint8_t mv_count;
    bool valid;
    bool auto_tune_completed;
    FbPidStatus_t status;
    int32_t backend_error; /* CKLIB error value; zero on success */
} FbPidOutput_t;

/* CKLIB-backed implementation. CKLIB types remain private to FbPid.c.
 * Current CKLIB uses 0.1 C: PV/SV are rounded once in the L04 backend.
 * TODO(Colin, next CKLIB version): unify backend to 0.001 C and remove the
 * temporary conversion after version/capability verification. MV stays 0.1%.
 * SetParameter units are NOT changed by the PV/SV pin conversion. */
typedef struct
{
    void *controller;
    void *auto_tune;
    FbPidConfig_t config;
    FbPidOutput_t output;
    uint32_t callback_count;
    uint32_t last_elapsed_ms;
    FbPidStatus_t last_status;
    int32_t backend_error;
    bool initialized;
    bool active;
    bool reset_input_previous;
    bool auto_tune_completed;
#if FB_PID_TEST_BACKEND
    int32_t test_mv; /* Fixed MV only; ABI requires one build-wide macro value. */
#endif
} FbPid_t;

#if FB_PID_TEST_BACKEND
/* Test backend uses no CKLIB/heap and only accepts PID + HEAT configuration.
 * Applies next Execute; does not implement tuning or numerical PID parameters. */
FbPidStatus_t FbPid_SetTestMv(FbPid_t *instance, int32_t mv);
#endif

FbPidStatus_t FbPid_Init(FbPid_t *instance, const FbPidConfig_t *config);
void FbPid_Deinit(FbPid_t *instance);
FbPidStatus_t FbPid_Reset(FbPid_t *instance);
FbPidStatus_t FbPid_SetParameter(FbPid_t *instance,
                                 FbPidParameter_t parameter,
                                 int32_t value);
FbPidStatus_t FbPid_Execute(FbPid_t *instance,
                            const FbPidInput_t *input,
                            FbPidOutput_t *output);

#ifdef __cplusplus
}
#endif

#endif /* FB_PID_H */
