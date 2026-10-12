#ifndef CKLIB_CK_ERROR_H_
#define CKLIB_CK_ERROR_H_

/**
 * @file ck_error.h
 * @brief Defines error codes and related utilities for the cklib.
 *
 * This header provides the standard error codes used throughout the library,
 * a macro for checking success, and a function to convert error codes to
 * human-readable strings.
 */

#include "cklib/inc/ck_define.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumerates the standard error codes for the cklib.
 */
enum CK_ERROR_LIST {
    /** @brief Indicates that the operation completed successfully. */
    CK_NO_ERR = 0,

    /** @brief Indicates that an invalid parameter was provided. */
    CK_ERROR_INVALID_PARAMETER = INT32_MIN,

    /** @brief Indicates that a feature or function is not yet implemented. */
    CK_ERROR_NOT_IMPLEMENTED,

    /** @brief Indicates that the auto-tune process failed. */
    CK_ERROR_AUTOTUNE_FAIL,
};

/**
 * @brief Defines the data type for error codes.
 *
 * This type is used as the return value for most functions in the library
 * to indicate the status of the operation.
 */
typedef int32_t CK_ERROR;

/**
 * @brief Checks if an error code represents a successful operation.
 *
 * @param error_code The CK_ERROR code to check.
 * @return true if the error code indicates success, false otherwise.
 */
#define CK_SUCCESS(error_code)      ((CK_ERROR)error_code >= CK_NO_ERR)

/**
 * @brief Converts a CK_ERROR code to a human-readable string.
 *
 * This function is useful for logging and debugging purposes.
 *
 * @param e The error code to convert.
 * @return A constant string representation of the error code. Returns
 *         "Unknown CK_ERROR" if the code is not recognized.
 */
CKLIB_EXPORT_IMPORT const char *CK_CALLTYPE ckErrorToString(CK_ERROR e);

#ifdef __cplusplus
}
#endif

#endif  // CKLIB_CK_ERROR_H_
