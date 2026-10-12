#ifndef CKLIB_DT_AUTO_TUNE_H_
#define CKLIB_DT_AUTO_TUNE_H_

/**
 * @file dt_auto_tune.h
 * @brief Defines the interface for a discrete-time auto-tune algorithm.
 *
 * This header provides the necessary structures, enumerations, and function
 * prototypes to create and manage an auto-tune instance.
 */

#include "cklib/inc/ck_auto_tune.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Creates an instance of a discrete-time auto-tune algorithm.
 *
 * This function allocates and initializes a new auto-tune object.
 * The returned auto-tuner must be freed using ckAutoTuneFree() when it is
 * no longer needed to prevent memory leaks.
 *
 * @return A handle to the newly created auto-tuner.
 *         Returns NULL if the auto-tuner could not be created (e.g., due to
 *         insufficient memory).
 * @see ckAutoTuneFree()
 */
CK_AUTO_TUNE CK_C_API ckCreateDtAutoTune();

/**
 * @brief Integer parameters for the auto-tune algorithm.
 */
enum DT_AUTO_TUNE_INT_PARAM{
    /**
     * @brief The setpoint (SV) for the auto-tune process.
     * @details This is the target value that the auto-tuner will attempt to
     *          oscillate around to determine the system's characteristics.
     */
    DT_AUTO_TUNE_PARAM_SV = 0x0000,
};

/**
 * @brief Unsigned integer parameters for the auto-tune algorithm.
 */
enum DT_AUTO_TUNE_UINT_PARAM {
    /**
     * @brief Setpoint ratio for tuning.
     * @details A scaling factor applied to the setpoint during the auto-tune
     *          process to adjust the aggressiveness of the test.
     *          Unit: percentage, Default: 100.
     */
    DT_AUTO_TUNE_PARAM_SV_RATIO,

    /**
     * @brief Proportional Band (PB) ratio.
     * @details A scaling factor applied to the calculated Proportional Band (PB)
     *          result. Allows for manual adjustment of the tuning result's
     *          aggressiveness.
     *          Unit: percentage, Default: 100.
     */
    DT_AUTO_TUNE_PARAM_PB_RATIO,

    /**
     * @brief Integral Time (Ti) ratio.
     * @details A scaling factor applied to the calculated Integral Time (Ti)
     *          result. Allows for manual adjustment of the tuning result.
     *          Unit: percentage, Default: 100.
     */
    DT_AUTO_TUNE_PARAM_TI_RATIO,

    /**
     * @brief Upper limit for the primary manipulated variable (MV1) during tuning.
     * @details Informs the auto-tuner about the maximum output power available,
     *          which can affect the calculated PID parameters.
     *          Range: 0 to 1000, representing 0% to 100.0%.
     */
    DT_AUTO_TUNE_PARAM_MV1_LIMIT_HIGH,

    /**
     * @brief Upper limit for the secondary manipulated variable (MV2) during tuning.
     * @details Used for the cooling output in Heat/Cool mode.
     *          Range: 0 to 1000, representing 0% to 100.0%.
     */
    DT_AUTO_TUNE_PARAM_MV2_LIMIT_HIGH,
};

/**
 * @brief Enumeration parameters for the auto-tune algorithm.
 */
enum DT_AUTO_TUNE_ENUM_PARAM {
    /**
     * @brief Control action for the target controller.
     * @details Informs the auto-tuner whether it should tune for heating,
     *          cooling, or heat/cool operation.
     * @see CK_CONTROL_ACTION
     */
    DT_AUTO_TUNE_PARAM_CONTROL_ACTION = 0x1000,
};

/**
 * @brief Boolean parameters for the auto-tune algorithm.
 */
enum DT_AUTO_TUNE_BOOL_PARAM {
    /**
     * @brief Disturbance compensation switch.
     * @details Informs the auto-tuner that the target controller has disturbance
     *          compensation enabled, which may result in less aggressive
     *          PID parameters.
     */
    DT_AUTO_TUNE_PARAM_DIS_COMP_SWITCH = 0x2000,
};

#ifdef __cplusplus
}
#endif

#endif  // CKLIB_DT_AUTO_TUNE_H_
