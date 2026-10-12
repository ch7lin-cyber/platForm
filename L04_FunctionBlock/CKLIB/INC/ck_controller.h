#ifndef CKLIB_CK_CONTROLLER_H_
#define CKLIB_CK_CONTROLLER_H_

/**
 * @file ck_controller.h
 * @brief Defines the base interface for all controllers in cklib.
 *
 * This file provides the core structures and function prototypes that
 * define a generic controller. All specific controller implementations
 * (e.g., PID, On-Off) should conform to this interface.
 */

#include "cklib/inc/ck_control_type.h"
#include "cklib/inc/ck_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Defines the data type for the Manipulated Variable (MV).
 * @details The unit for this variable is 0.1 degrees.
 */
typedef int32_t control_value_t;

/**
 * @brief Represents the output of a controller.
 *
 * This structure holds the calculated Manipulated Variable(s) (MV)
 * that the controller determines should be applied to the system.
 */
typedef struct _CK_OUTPUT {
    /**
     * @brief Array of Manipulated Variables.
     * @details Depending on the control action (e.g., heat-only, heat/cool),
     *          this array can hold one or two values.
     */
    control_value_t mv[2];

    /**
     * @brief The number of valid MV values in the `mv` array.
     * @details This will be 1 for single-action controllers (like heat-only)
     *          and can be up to 2 for dual-action controllers (like heat/cool).
     */
    size_t mv_count;
} CK_OUTPUT;

/**
 * @brief An opaque handle to a generic controller object.
 *
 * This handle represents a controller instance and is used in all
 * controller-related API functions.
 */
typedef struct _CK_CONTROLLER *CK_CONTROLLER;

/**
 * @brief Defines the data type for the control variable type.
 *
 * This type is used to specify the variable of the control algorithm to be
 * get/set with.
 */
typedef int32_t CK_CONTROLLER_PARAM;

/**
 * @brief Performs a single control step.
 *
 * This function executes one iteration of the control algorithm. It updates
 * the controller's internal state (e.g., error accumulation) and calculates
 * the new output Manipulated Variable(s) based on the current process
 * values and setpoints.
 *
 * @param controller A handle to the controller instance.
 * @param[out] output A pointer to a CK_OUTPUT structure to store the calculated
 *                    manipulated variables.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckControllerControl(CK_CONTROLLER controller,
                                      CK_OUTPUT *output);

/**
 * @brief Resets the internal state of the controller.
 *
 * This function clears any accumulated values, such as integral error,
 * effectively resetting the controller to its initial state.
 *
 * @param controller A handle to the controller instance.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckControllerReset(CK_CONTROLLER controller);

/**
 * @brief Gets the type of the controller.
 *
 * @param controller A handle to the controller instance.
 * @param[out] type A pointer to store the controller type, as defined in
 *                  #CK_CONTROL_TYPE_LIST.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckControllerGetType(CK_CONTROLLER controller,
                                      CK_CONTROL_TYPE *type);

/**
 * @brief Frees a controller instance and its associated resources.
 *
 * This function should be called to release the memory allocated for a
 * controller object when it is no longer needed.
 *
 * @param controller A handle to the controller instance to be freed.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckControllerFree(CK_CONTROLLER controller);

/**
 * @brief Sets the Process Value (PV) for the controller.
 *
 * @param controller A handle to the controller instance.
 * @param value The current process value.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckControllerSetPV(CK_CONTROLLER controller, int32_t value);

/**
 * @brief Sets the Set Value (SV) for the controller.
 *
 * @param controller A handle to the controller instance.
 * @param value The target setpoint value.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckControllerSetSV(CK_CONTROLLER controller, int32_t value);

/**
 * @brief Sets the step time for the controller.
 *
 * @param controller A handle to the controller instance.
 * @param value The step time value. (Unit: 10ms)
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckControllerSetStepTime(CK_CONTROLLER controller,
                                          int32_t value);

/**
 * @brief Gets an integer parameter from the controller.
 *
 * @param controller A handle to the controller instance.
 * @param type The identifier of the parameter to be accessed with.
 * @param[out] value A pointer to store the retrieved integer value.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckControllerGetIntParam(CK_CONTROLLER controller,
                                          CK_CONTROLLER_PARAM type,
                                          int32_t *value);

/**
 * @brief Sets an integer parameter for the controller.
 *
 * @param controller A handle to the controller instance.
 * @param type The identifier of the parameter to be accessed with.
 * @param value The integer value to set for the parameter.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckControllerSetIntParam(CK_CONTROLLER controller,
                                          CK_CONTROLLER_PARAM type,
                                          int32_t value);

/**
 * @brief Gets an unsigned integer parameter from the controller.
 *
 * @param controller A handle to the controller instance.
 * @param type The identifier of the parameter to be accessed with.
 * @param[out] value A pointer to store the retrieved unsigned integer value.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckControllerGetUIntParam(CK_CONTROLLER controller,
                                           CK_CONTROLLER_PARAM type,
                                           uint32_t *value);

/**
 * @brief Sets an unsigned integer parameter for the controller.
 *
 * @param controller A handle to the controller instance.
 * @param type The identifier of the parameter to be accessed with.
 * @param value The unsigned integer value to set for the parameter.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckControllerSetUIntParam(CK_CONTROLLER controller,
                                           CK_CONTROLLER_PARAM type,
                                           uint32_t value);

/**
 * @brief Gets a boolean parameter from the controller.
 *
 * @param controller A handle to the controller instance.
 * @param type The identifier of the parameter to be accessed with.
 * @param[out] value A pointer to store the retrieved boolean value.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckControllerGetBoolParam(CK_CONTROLLER controller,
                                           CK_CONTROLLER_PARAM type,
                                           bool *value);

/**
 * @brief Sets a boolean parameter for the controller.
 *
 * @param controller A handle to the controller instance.
 * @param type The identifier of the parameter to be accessed with.
 * @param value The boolean value to set for the parameter.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckControllerSetBoolParam(CK_CONTROLLER controller,
                                           CK_CONTROLLER_PARAM type,
                                           bool value);
/**
 * @brief Gets an enumeration parameter from the controller.
 *
 * @param controller A handle to the controller instance.
 * @param type The identifier of the parameter to be accessed with.
 * @param[out] value A pointer to store the retrieved enumeration value.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckControllerGetEnumParam(CK_CONTROLLER controller,
                                           CK_CONTROLLER_PARAM type,
                                           int32_t *value);

/**
 * @brief Sets an enumeration parameter for the controller.
 *
 * @param controller A handle to the controller instance.
 * @param type The identifier of the parameter to be accessed with.
 * @param value The enumeration value to set for the parameter.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckControllerSetEnumParam(CK_CONTROLLER controller,
                                           CK_CONTROLLER_PARAM type,
                                           int32_t value);

/**
 * @brief Enumerates the possible control actions for a controller.
 *
 * This enumeration defines the behavior of the controller's output(s).
 */
enum CK_CONTROL_ACTION_LIST {
    /**
     * @brief Heat-only control action.
     * @details The controller provides a single output for heating,
     *          typically stored in `mv[0]`.
     */
    CK_CONTROL_ACTION_HEAT = 0,

    /**
     * @brief Cool-only control action.
     * @details The controller provides a single output for cooling,
     *          typically stored in `mv[0]`.
     */
    CK_CONTROL_ACTION_COOL = 1,

    /**
     * @brief Heat and Cool control action.
     * @details The controller provides two outputs. `mv[0]` is used for
     *          heating, and `mv[1]` is used for cooling.
     */
    CK_CONTROL_ACTION_HEAT_COOL = 2,
};

/**
 * @brief Defines the data type for the control action.
 *
 * This type is used to specify the controller's output behavior, using
 * values from #CK_CONTROL_ACTION_LIST.
 */
typedef uint8_t CK_CONTROL_ACTION;

#ifdef __cplusplus
}
#endif

#endif  // CKLIB_CK_CONTROLLER_H_
