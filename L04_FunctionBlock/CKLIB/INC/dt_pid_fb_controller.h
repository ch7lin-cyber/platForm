#ifndef CKLIB_DT_PID_FB_CONTROLLER_H_
#define CKLIB_DT_PID_FB_CONTROLLER_H_

#include "cklib/inc/ck_controller.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Creates an instance of a discrete-time PID controller (FB variant).
 *
 * The returned controller must be freed using ckControllerFree().
 */
CK_CONTROLLER CK_C_API ckCreateDtPidFbController();

#ifdef __cplusplus
}
#endif

#endif  // CKLIB_DT_PID_FB_CONTROLLER_H_

