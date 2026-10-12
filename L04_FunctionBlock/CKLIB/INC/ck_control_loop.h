#ifndef CKLIB_CK_CONTROL_LOOP_H_
#define CKLIB_CK_CONTROL_LOOP_H_

#include "cklib/inc/ck_controller.h"
#include "cklib/inc/ck_auto_tune.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CK_CONTROL_LOOP CK_CONTROL_LOOP;
typedef CK_CONTROL_LOOP *CK_CONTROL_LOOP_HANDLE;

/**
 Lists the supported gating states for the supervisor.
 */
enum CK_CONTROL_LOOP_GATING_STATE_LIST {
    CK_CONTROL_LOOP_GATING_FAIL = 0,
    CK_CONTROL_LOOP_GATING_STANDBY,
    CK_CONTROL_LOOP_GATING_MANUAL,
    CK_CONTROL_LOOP_GATING_AUTO,
};

/**
 Defines the data type for the gating state.
 */
typedef int32_t CK_CONTROL_LOOP_GATING_STATE;

/**
 Lists the simplified FSM events handled by the control loop.
 *
 * Detection priority follows the basic thermostat state machine:
 * 1. input fail
 * 2. run/standby
 * 3. auto/manual
 */
enum CK_CONTROL_LOOP_FSM_EVENT_LIST {
    CK_CONTROL_LOOP_FSM_EVENT_NONE = 0,
    CK_CONTROL_LOOP_FSM_EVENT_INPUT_FAIL_ENTER,
    CK_CONTROL_LOOP_FSM_EVENT_INPUT_FAIL_EXIT,
    CK_CONTROL_LOOP_FSM_EVENT_RUN_STOP,
    CK_CONTROL_LOOP_FSM_EVENT_RUN_START,
    CK_CONTROL_LOOP_FSM_EVENT_AUTO_EXIT,
    CK_CONTROL_LOOP_FSM_EVENT_AUTO_ENTER,
};
typedef int32_t CK_CONTROL_LOOP_FSM_EVENT;

/**
 Shared pool values written by the app/ISR and read by the supervisor.
 */
typedef struct _CK_CONTROL_LOOP_POOL {
    control_value_t pv;
    control_value_t sv;

    CK_OUTPUT manual_mv;
    CK_OUTPUT standby_mv;
    CK_OUTPUT input_fail_mv;

    bool run;
    bool auto_mode;
    bool input_ok;
    int32_t control_mode;

    const void *params_blob_ptr;
    size_t params_blob_size;

    CK_OUTPUT last_mv_out;

    uint32_t sv_commit_seq;
    uint32_t param_commit_seq;
    uint32_t control_mode_commit_seq;
} CK_CONTROL_LOOP_POOL;

/**
 Maps an external control mode value to a controller instance.
 */
typedef struct _CK_CONTROL_LOOP_CONTROLLER_SLOT {
    int32_t control_mode;
    CK_CONTROLLER controller;
} CK_CONTROL_LOOP_CONTROLLER_SLOT;

/**
 Defines the few configurable reset behaviors used by the supervisor.
 */
typedef struct _CK_CONTROL_LOOP_POLICY {
    bool reset_on_enter_auto;
    bool reset_on_leave_auto;
    bool reset_on_sv_commit;
    bool reset_on_param_commit;
    bool reset_on_control_mode_change;
} CK_CONTROL_LOOP_POLICY;

/**
 Describes the resources required to create a control loop.
 */
typedef struct _CK_CONTROL_LOOP_CONFIG {
    CK_CONTROL_LOOP_POOL *pool;
    const CK_CONTROL_LOOP_CONTROLLER_SLOT *controllers;
    size_t controller_count;
    const CK_CONTROL_LOOP_POLICY *policy;
    CK_AUTO_TUNE autotune;
    CK_CONTROLLER at_target_controller;
    int32_t control_mode_at;
    int32_t control_mode_after_at;
} CK_CONTROL_LOOP_CONFIG;

/**
 Control loop object
 */
struct _CK_CONTROL_LOOP {
    CK_CONTROL_LOOP_POOL *pool;
    CK_CONTROL_LOOP_POOL snapshot;

    const CK_CONTROL_LOOP_CONTROLLER_SLOT *controllers;
    size_t controller_count;

    CK_CONTROL_LOOP_POLICY policy;

    CK_CONTROL_LOOP_GATING_STATE gating_state_prev;
    CK_CONTROL_LOOP_GATING_STATE gating_state_now;
    CK_CONTROL_LOOP_FSM_EVENT last_fsm_event;

    uint32_t last_seen_sv_commit_seq;
    uint32_t last_seen_param_commit_seq;
    uint32_t last_seen_control_mode_commit_seq;
    int32_t control_mode_prev;

    CK_CONTROLLER active_controller;
    CK_AUTO_TUNE autotune;
    CK_CONTROLLER at_target_controller;
    int32_t control_mode_at;
    int32_t control_mode_after_at;
};











/**
 * @brief Initializes a policy structure with the library defaults.
 *
 * @param[out] policy The policy structure to initialize.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckControlLoopInitPolicyDefaults(CK_CONTROL_LOOP_POLICY *policy);

/**
 * @brief Initializes a caller-owned control loop supervisor instance.
 *
 * The caller owns the pool, controller instances, and controller mapping table.
 *
 * @param[out] loop The storage of the control loop instance.
 * @param config The creation parameters for the control loop.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckControlLoopInit(CK_CONTROL_LOOP *loop, const CK_CONTROL_LOOP_CONFIG *config);

/**
 * @brief Deinitializes a caller-owned control loop supervisor instance.
 *
 * @param loop The control loop to free.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckControlLoopDeinit(CK_CONTROL_LOOP *loop);

/**
 * @brief Creates a heap-backed control loop supervisor instance.
 *
 * This is a host-side convenience wrapper around ckControlLoopInit().
 *
 * @param config The creation parameters for the control loop.
 * @return The created control loop handle, or NULL on failure.
 */
CK_CONTROL_LOOP_HANDLE CK_C_API ckCreateControlLoop(const CK_CONTROL_LOOP_CONFIG *config);

/**
 * @brief Frees a heap-backed control loop instance created by ckCreateControlLoop().
 *
 * @param loop The control loop to free.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckControlLoopFree(CK_CONTROL_LOOP *loop);

/**
 * @brief Executes one deterministic supervisor tick.
 *
 * @param loop The control loop handle.
 * @param step_time The control step time in 10ms units.
 * @param[out] output The final routed MV output for this tick.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckControlLoopTick(CK_CONTROL_LOOP *loop ,int32_t step_time ,CK_OUTPUT *output);

/**
 * @brief Gets the current gating state observed by the control loop.
 *
 * @param loop The control loop handle.
 * @param[out] state The resulting gating state.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckControlLoopGetGatingState(
    CK_CONTROL_LOOP *loop,
    CK_CONTROL_LOOP_GATING_STATE *state);

/**
 * @brief Gets the FSM event detected during the most recent tick.
 *
 * @param loop The control loop handle.
 * @param[out] event The detected FSM event.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckControlLoopGetLastFsmEvent(
    CK_CONTROL_LOOP *loop,
    CK_CONTROL_LOOP_FSM_EVENT *event);

#ifdef __cplusplus
}
#endif

#endif  // CKLIB_CK_CONTROL_LOOP_H_
