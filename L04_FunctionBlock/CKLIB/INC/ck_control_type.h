#ifndef CKLIB_CK_CONTROL_TYPE_H_
#define CKLIB_CK_CONTROL_TYPE_H_

/**
 * @file ck_control_type.h
 * @brief Defines the control types for the cklib.
 */

#include "cklib/inc/ck_error.h"

/**
 * @brief Lists the available control types.
 *
 * Each control type corresponds to a different control strategy.
 * The control type is a key parameter within the control configuration.
 */
enum CK_CONTROL_TYPE_LIST {
    /** @brief Undefined control type */
    CK_CONTROL_TYPE_UNDEFINED = 0,

    /** @brief Proportional-Integral-Derivative (PID) control */
    CK_CONTROL_TYPE_PID,

    /** @brief On-Off or hysteresis control */
    CK_CONTROL_TYPE_ON_OFF,
};

/**
 * @brief Defines the data type for the control type.
 *
 * This type is used to specify the control algorithm to be used.
 */
typedef int32_t CK_CONTROL_TYPE;

#endif  // CKLIB_CK_CONTROL_TYPE_H_
