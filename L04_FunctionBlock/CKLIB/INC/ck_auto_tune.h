#ifndef CKLIB_CK_AUTO_TUNE_H_
#define CKLIB_CK_AUTO_TUNE_H_

/**
 * @file ck_auto_tune.h
 * @brief Defines the base interface for auto-tune algorithms in cklib.
 *
 * This file provides the core structures and function prototypes that
 * define a generic auto-tuner. Specific auto-tune implementations should
 * conform to this interface.
 */

#include "cklib/inc/ck_controller.h"
#include "cklib/inc/ck_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief An opaque handle to a generic auto-tune object.
 *
 * This handle represents an auto-tuner instance and is used in all
 * auto-tune related API functions.
 */
typedef struct _CK_AUTO_TUNE *CK_AUTO_TUNE;

// -----------------------------------------------------------------------------

/**
 * @brief Performs one step of the auto-tune data collection and signal generation.
 *
 * This function is called iteratively during the auto-tune process. It takes the
 * current process value (PV) as input, updates the auto-tuner's internal state,
 * and generates an output signal (MV) to perturb the system and observe its
 * response.
 *
 * @param autotune A handle to the auto-tuner instance.
 * @param pv The current process value.
 * @param[out] output A pointer to a CK_OUTPUT structure to store the calculated
 *                    manipulated variable. The `mv_count` will be 0 when the
 *                    auto-tune process is complete.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckAutoTuneCollectAndSignal(CK_AUTO_TUNE autotune,
                                             control_value_t pv,
                                             CK_OUTPUT *output);

/**
 * @brief Applies the parameters calculated by the auto-tuner to a controller.
 *
 * After a successful call to ckAutoTuneCalculate(), this function applies the
 * newly found parameters to a specified controller instance.
 *
 * @param autotune A handle to the auto-tuner instance containing the calculated
 *                 parameters.
 * @param controller A handle to the controller instance to apply parameters to.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckAutoTuneApplyParams(CK_AUTO_TUNE autotune,
                                        CK_CONTROLLER controller);

/**
 * @brief Resets the internal state of the auto-tuner.
 *
 * This function clears any collected data and resets the auto-tuner to its
 * initial state, preparing it for a new tuning session.
 *
 * @param autotune A handle to the auto-tuner instance.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckAutoTuneReset(CK_AUTO_TUNE autotune);

/**
 * @brief Frees an auto-tuner instance and its associated resources.
 *
 * This function should be called to release the memory allocated for an
 * auto-tuner object when it is no longer needed.
 *
 * @param autotune A handle to the auto-tuner instance to be freed.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckAutoTuneFree(CK_AUTO_TUNE autotune);

/**
 * @brief Defines the data type for the auto-tune parameter type.
 *
 * This type is used to specify the parameter of the auto-tune algorithm to be
 * get/set.
 */
typedef int32_t CK_AUTO_TUNE_PARAM;


 /**
  * @brief Gets an integer parameter from the auto-tuner.
  *
  * @param autotune A handle to the auto-tuner instance.
  * @param type The identifier of the parameter to get.
  * @param[out] value A pointer to store the retrieved integer value.
  * @return CK_NO_ERR on success, or an error code on failure.
  */
CK_ERROR CK_C_API ckAutoTuneGetIntParam(CK_AUTO_TUNE autotune,
                                        CK_AUTO_TUNE_PARAM type,
                                        int32_t *value);

/**
 * @brief Sets an integer parameter for the auto-tuner.
 *
 * @param autotune A handle to the auto-tuner instance.
 * @param type The identifier of the parameter to set.
 * @param value The integer value to set for the parameter.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckAutoTuneSetIntParam(CK_AUTO_TUNE autotune,
                                        CK_AUTO_TUNE_PARAM type,
                                        int32_t value);

/**
 * @brief Gets an unsigned integer parameter from the auto-tuner.
 *
 * @param autotune A handle to the auto-tuner instance.
 * @param type The identifier of the parameter to get.
 * @param[out] value A pointer to store the retrieved unsigned integer value.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckAutoTuneGetUIntParam(CK_AUTO_TUNE autotune,
                                         CK_AUTO_TUNE_PARAM type,
                                         uint32_t *value);

/**
 * @brief Sets an unsigned integer parameter for the auto-tuner.
 *
 * @param autotune A handle to the auto-tuner instance.
 * @param type The identifier of the parameter to set.
 * @param value The unsigned integer value to set for the parameter.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckAutoTuneSetUIntParam(CK_AUTO_TUNE autotune,
                                         CK_AUTO_TUNE_PARAM type,
                                         uint32_t value);

/**
 * @brief Gets a boolean parameter from the auto-tuner.
 *
 * @param autotune A handle to the auto-tuner instance.
 * @param type The identifier of the parameter to get.
 * @param[out] value A pointer to store the retrieved boolean value.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckAutoTuneGetBoolParam(CK_AUTO_TUNE autotune,
                                         CK_AUTO_TUNE_PARAM type,
                                         bool *value);

/**
 * @brief Sets a boolean parameter for the auto-tuner.
 *
 * @param autotune A handle to the auto-tuner instance.
 * @param type The identifier of the parameter to set.
 * @param value The boolean value to set for the parameter.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckAutoTuneSetBoolParam(CK_AUTO_TUNE autotune,
                                         CK_AUTO_TUNE_PARAM type,
                                         bool value);

/**
 * @brief Gets an enumeration parameter from the auto-tuner.
 *
 * @param autotune A handle to the auto-tuner instance.
 * @param type The identifier of the parameter to get.
 * @param[out] value A pointer to store the retrieved enumeration value.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckAutoTuneGetEnumParam(CK_AUTO_TUNE autotune,
                                         CK_AUTO_TUNE_PARAM type,
                                         int32_t *value);

/**
 * @brief Sets an enumeration parameter for the auto-tuner.
 *
 * @param autotune A handle to the auto-tuner instance.
 * @param type The identifier of the parameter to set.
 * @param value The enumeration value to set for the parameter.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckAutoTuneSetEnumParam(CK_AUTO_TUNE autotune,
                                         CK_AUTO_TUNE_PARAM type,
                                         int32_t value);

/**
 * @brief Sets the setpoint (SV) for the auto-tune process.
 *
 * @param autotune A handle to the auto-tuner instance.
 * @param value The target setpoint value.
 * @return CK_NO_ERR on success, or an error code on failure.
 */
CK_ERROR CK_C_API ckAutoTuneSetSV(CK_AUTO_TUNE autotune, int32_t value);

#ifdef __cplusplus
}
#endif

#endif  // CKLIB_CK_AUTO_TUNE_H_
