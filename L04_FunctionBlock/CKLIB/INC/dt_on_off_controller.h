#ifndef CKLIB_DT_ON_OFF_CONTROLLER_H_
#define CKLIB_DT_ON_OFF_CONTROLLER_H_

/**
 * @file dt_on_off_controller.h
 * @brief Defines the interface for a discrete-time ON-OFF controller.
 *
 * This header provides the necessary structures, enumerations, and function
 * prototypes to create and manage an ON-OFF controller.
 *
 * The behavior of the ON-OFF control depends on the control action set:
 * - `CK_CONTROL_ACTION_HEAT_COOL`:
 *   When `(SV - DEAD_BAND / 2 - OUT1_TS) <= PV <= (SV + DEAD_BAND / 2 + OUT2_TS)`,
 *   the outputs (mv) are 0.
 * - `CK_CONTROL_ACTION_HEAT` / `CK_CONTROL_ACTION_COOL`:
 *   When the difference between PV and SV is less than OUT1_TS, the output (mv) is 0.
 */

#include "cklib/inc/ck_controller.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Creates an instance of a discrete-time ON-OFF controller.
 *
 * This function allocates and initializes a new ON-OFF controller object.
 * The returned controller must be freed using ckControllerFree() when it is
 * no longer needed to prevent memory leaks.
 *
 * @return A handle to the newly created ON-OFF controller.
 *         Returns NULL if the controller could not be created (e.g., due to
 *         insufficient memory).
 * @see ckControllerFree()
 */
CK_CONTROLLER CK_C_API ckCreateDtOnOffController();

/**
 * @brief Integer parameters for the ON-OFF controller.
 */
enum DT_ON_OFF_INT_PARAM {
    /**
     * @brief Process Value: actual measured temperature.
     * @details The unit for this variable is 0.1 degrees.
     */
    DT_ON_OFF_PARAM_PV,

    /**
     * @brief Set Value: setting temperature.
     * @details The unit for this variable is 0.1 degrees.
     */
    DT_ON_OFF_PARAM_SV,

    /**
     * @brief Sensitivity of output 1.
     * @details Also known as hysteresis or switching differential for output 1.
     */
    DT_ON_OFF_PARAM_OUT1_TS,

    /**
     * @brief Sensitivity of output 2.
     * @details Also known as hysteresis or switching differential for output 2.
     */
    DT_ON_OFF_PARAM_OUT2_TS,

    /**
     * @brief Output deadband.
     * @details This parameter is only effective when the control action is
     *          `CK_CONTROL_ACTION_HEAT_COOL`. It defines a neutral zone
     *          around the setpoint where no control action is taken.
     */
    DT_ON_OFF_PARAM_DEADBAND,
};

/**
 * @brief Enumeration parameters for the ON-OFF controller.
 */
enum DT_ON_OFF_ENUM_PARAM {
    /**
     * @brief Control output behavior.
     * @details This parameter determines the control action, e.g., heating,
     *          cooling, or both. See values in CK_CONTROL_ACTION.
     */
    DT_ON_OFF_PARAM_CONTROL_ACTION = 0x1000,
};

#ifdef __cplusplus
}
#endif

#endif  // CKLIB_DT_ON_OFF_CONTROLLER_H_
