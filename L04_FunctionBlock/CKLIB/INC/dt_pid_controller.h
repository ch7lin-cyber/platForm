#ifndef CKLIB_DT_PID_CONTROLLER_H_
#define CKLIB_DT_PID_CONTROLLER_H_

/**
 * @file dt_pid_controller.h
 * @brief Defines the interface for a discrete-time PID controller.
 *
 * This header provides the necessary structures, enumerations, and function
 * prototypes to create and manage a PID controller.
 */

#include "cklib/inc/ck_controller.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Creates an instance of a discrete-time PID controller.
 *
 * This function allocates and initializes a new PID controller object.
 * The returned controller must be freed using ckControllerFree() when it is
 * no longer needed to prevent memory leaks.
 *
 * @return A handle to the newly created PID controller.
 *         Returns NULL if the controller could not be created (e.g., due to
 *         insufficient memory).
 * @see ckControllerFree()
 */
CK_CONTROLLER CK_C_API ckCreateDtPidController();

/**
 * @brief Integer parameters for the PID controller.
 */
enum DT_PID_INT_PARAM {
    /**
     * @brief Process Value: actual measured temperature.
     * @details The unit for this variable is 0.1 degrees.
     */
    DT_PID_PARAM_PV,

    /**
     * @brief Set Value: setting temperature.
     * @details The unit for this variable is 0.1 degrees.
     */
    DT_PID_PARAM_SV,

    /**
     * @brief Output deadband.
     * @details This parameter is only effective when the control action is
     *          `CK_CONTROL_ACTION_HEAT_COOL`. It defines a neutral zone
     *          around the setpoint where no control action is taken.
     */
    DT_PID_PARAM_DEADBAND,

    /**
     * @brief Derivative action limit gate.
     * @details The derivative action is suppressed when the error exceeds this
     *          gate value, preventing large output spikes on sudden setpoint
     *          changes.
     */
    DT_PID_PARAM_D_LIMIT_GATE,

    /**
     * @brief Stability range for the process variable.
     * @details Defines a range around the setpoint within which the process
     *          variable is considered stable. Used by features like disturbance
     *          compensation.
     */
    DT_PID_PARAM_STABLE_RANGE,

    /**
     * @brief Error signal.
     * @details The error signal is the difference between the setpoint and the process variable.
     */
    DT_PID_PARAM_ERR,

    /**
     * @brief Set Value (2DOF).
     * @details The 2DOF SV value for the controller.
     */
    DT_PID_PARAM_SV_2DOF,

    /**
     * @brief The calculated value of the derivative term.
     * @details This is a read-only parameter providing insight into the
     *          controller's internal state.
     */
    DT_PID_PARAM_D_VALUE,

    /**
     * @brief The accumulated value of the integral term.
     * @details This is a read-only parameter useful for monitoring integral
     *          windup or for debugging the controller's behavior.
     */
    DT_PID_PARAM_INTEGRAL_VALUE,
};

/**
 * @brief Unsigned integer parameters for the PID controller.
 */
enum DT_PID_UINT_PARAM  {
    /**
     * @brief Proportional Band for the primary (heating) output.
     * @details A smaller value results in a more aggressive proportional action.
     */
    DT_PID_PARAM_PB = 0x1000,

    /**
     * @brief Integral Time for the primary (heating) output.
     * @details The time constant for the integral action, in seconds. A smaller
     *          value results in a faster integral response.
     */
    DT_PID_PARAM_TI,

    /**
     * @brief Derivative Time for the primary (heating) output.
     * @details The time constant for the derivative action, in seconds. A larger
     *          value results in a stronger derivative response.
     */
    DT_PID_PARAM_TD,

    /**
     * @brief Proportional Band for the cooling output.
     * @details Only used in Heat/Cool control mode.
     */
    DT_PID_PARAM_C_PB,

    /**
     * @brief Integral Time for the cooling output.
     * @details Only used in Heat/Cool control mode.
     */
    DT_PID_PARAM_C_TI,

    /**
     * @brief Derivative Time for the cooling output.
     * @details Only used in Heat/Cool control mode.
     */
    DT_PID_PARAM_C_TD,

    /**
     * @brief Upper limit for the primary manipulated variable (MV1).
     * @details Range: 0 to 1000, representing 0% to 100.0%.
     */
    DT_PID_PARAM_MV1_LIMIT_HIGH,

    /**
     * @brief Lower limit for the primary manipulated variable (MV1).
     * @details Range: 0 to 1000, representing 0% to 100.0%.
     */
    DT_PID_PARAM_MV1_LIMIT_LOW,

    /**
     * @brief Upper limit for the secondary manipulated variable (MV2).
     * @details Used for the cooling output in Heat/Cool mode.
     *          Range: 0 to 1000, representing 0% to 100.0%.
     */
    DT_PID_PARAM_MV2_LIMIT_HIGH,

    /**
     * @brief Lower limit for the secondary manipulated variable (MV2).
     * @details Used for the cooling output in Heat/Cool mode.
     *          Range: 0 to 1000, representing 0% to 100.0%.
     */
    DT_PID_PARAM_MV2_LIMIT_LOW,

    /**
     * @brief Alpha coefficient for the 2-DoF (Two-Degree-of-Freedom) setpoint filter.
     * @details Helps in reducing setpoint change overshoot.
     *          Unit: 0.01, Range: 1 to 100.
     */
    DT_PID_PARAM_ALPHA,

    /**
     * @brief Beta coefficient for the 2-DoF setpoint filter.
     * @details Helps in reducing setpoint change overshoot.
     *          Unit: 0.01, Range: 1 to 100.
     */
    DT_PID_PARAM_BETA,

    /**
     * @brief Gamma coefficient for the 2-DoF setpoint filter.
     * @details Helps in reducing setpoint change overshoot.
     *          Unit: 0.01, Range: 1 to 100.
     */
    DT_PID_PARAM_GAMMA,

    /**
     * @brief Integral preload value.
     * @details An initial value for the integral term to prevent a long wait
     *          for the integral action to build up at startup. The actual
     *          integral value is calculated as: `ipreload * pb * ti / 1000`.
     */
    DT_PID_PARAM_IPRELOAD,

    /**
     * @brief Proportional-Derivative Output Offset (Manual Reset).
     * @details An offset added to the output when the integral term is disabled
     *          (i.e., Ti = 0), effectively making it a PD controller.
     */
    DT_PID_PARAM_PDOF,

    /**
     * @brief The execution period of the PID controller.
     * @details Specifies the sampling time for the PID calculations.
     *          Unit: 10ms.
     */
    DT_PID_PARAM_PERIOD,
};

/**
 * @brief Enumeration parameters for the PID controller.
 */
enum DT_PID_ENUM_PARAM {
    /**
     * @brief Control output behavior.
     * @details This parameter determines the control action, e.g., heating,
     *          cooling, or both. See values in CK_CONTROL_ACTION.
     */
    DT_PID_PARAM_CONTROL_ACTION = 0x2000,

    /**
     * @brief PID operation mode.
     * @details Selects the PID calculation mode.
     *          - 0: Standard mode with 2-DoF setpoint filter.
     *          - 1: Fast mode, which bypasses the 2-DoF setpoint filter for a
     *               more direct response.
     */
    DT_PID_PARAM_PID_MODE,
};

/**
 * @brief Boolean parameters for the PID controller.
 */
enum DT_PID_BOOL_PARAM {
    /**
     * @brief Disturbance Compensation Switch.
     * @details Enables or disables the disturbance compensation feature. When
     *          enabled, the controller attempts to counteract detected
     *          disturbances to maintain stability.
     */
    DT_PID_PARAM_DIS_COMP_SWITCH = 0x3000,
};

#ifdef __cplusplus
}
#endif

#endif  // CKLIB_DT_PID_CONTROLLER_H_
