#include "FbPid.h"

#include <limits.h>
#include <stddef.h>
#include <string.h>

#include "cklib/inc/ck_auto_tune.h"
#include "cklib/inc/ck_controller.h"
#include "cklib/inc/dt_auto_tune.h"
#include "cklib/inc/dt_on_off_controller.h"
#include "cklib/inc/dt_pid_controller.h"
#include "cklib/inc/dt_pid_fb_controller.h"

/* Temporary compatibility boundary: public milli-C -> current CKLIB 0.1 C.
 * Round nearest; exact half away from zero. int64 avoids INT32_MIN overflow.
 * TODO(Colin, next CKLIB): backend unified to 0.001 C. Verify its actual API
 * version, then remove this conversion in BOTH controller and AutoTune paths.
 * Do not also divide in L05. This does not convert SetParameter values. */
static int32_t ToCklibTemperature(int32_t millicelsius)
{
    int64_t value=millicelsius;
    return (int32_t)(value>=0 ? (value+50)/100 : -((-value+50)/100));
}

static bool IsPidMode(FbPidMode_t mode)
{
    return mode == FB_PID_MODE_PID || mode == FB_PID_MODE_PID_FB;
}

static CK_CONTROLLER CreateController(FbPidMode_t mode)
{
    switch (mode)
    {
        case FB_PID_MODE_ON_OFF: return ckCreateDtOnOffController();
        case FB_PID_MODE_PID: return ckCreateDtPidController();
        case FB_PID_MODE_PID_FB: return ckCreateDtPidFbController();
        default: return NULL;
    }
}

static CK_CONTROLLER Controller(FbPid_t *instance)
{
    return (CK_CONTROLLER)instance->controller;
}

static CK_AUTO_TUNE AutoTune(FbPid_t *instance)
{
    return (CK_AUTO_TUNE)instance->auto_tune;
}

static void ClearOutput(FbPidOutput_t *output, FbPidStatus_t status)
{
    (void)memset(output, 0, sizeof(*output));
    output->status = status;
}

static CK_ERROR SetControlAction(CK_CONTROLLER controller,
                                 FbPidMode_t mode,
                                 FbPidControlAction_t action)
{
    CK_CONTROLLER_PARAM parameter = (mode == FB_PID_MODE_ON_OFF) ?
        DT_ON_OFF_PARAM_CONTROL_ACTION : DT_PID_PARAM_CONTROL_ACTION;
    return ckControllerSetEnumParam(controller, parameter, (int32_t)action);
}

static bool ValidConfig(const FbPidConfig_t *config)
{
    if (config == NULL || config->mode < FB_PID_MODE_AUTO_TUNE ||
        config->mode >= FB_PID_MODE_COUNT ||
        config->auto_tune_target_mode < FB_PID_MODE_AUTO_TUNE ||
        config->auto_tune_target_mode >= FB_PID_MODE_COUNT ||
        config->control_action < FB_PID_ACTION_HEAT ||
        config->control_action > FB_PID_ACTION_HEAT_COOL)
    {
        return false;
    }
    if (config->mode == FB_PID_MODE_AUTO_TUNE)
    {
        return IsPidMode(config->auto_tune_target_mode) &&
               config->auto_tune_period_ms >= 10U;
    }
    return config->mode != FB_PID_MODE_COUNT;
}

static bool ResetRuntime(FbPid_t *instance)
{
    CK_ERROR error = ckControllerReset(Controller(instance));
    if (CK_SUCCESS(error) && AutoTune(instance) != NULL)
    {
        error = ckAutoTuneReset(AutoTune(instance));
    }
    if (!CK_SUCCESS(error))
    {
        instance->backend_error = error;
        instance->active = false;
        ClearOutput(&instance->output, FB_PID_STATUS_BACKEND_ERROR);
        instance->output.backend_error = error;
        return false;
    }
    instance->active = false;
    instance->auto_tune_completed = false;
    instance->backend_error = CK_NO_ERR;
    ClearOutput(&instance->output, FB_PID_STATUS_RESET);
    return true;
}

FbPidStatus_t FbPid_Init(FbPid_t *instance, const FbPidConfig_t *config)
{
    CK_CONTROLLER controller;
    CK_AUTO_TUNE auto_tune = NULL;
    CK_ERROR error;
    FbPidMode_t controller_mode;

    if (instance == NULL || !ValidConfig(config))
    {
        return FB_PID_STATUS_INVALID_CONFIG;
    }
    (void)memset(instance, 0, sizeof(*instance));
    instance->config = *config;
    controller_mode = (config->mode == FB_PID_MODE_AUTO_TUNE) ?
                      config->auto_tune_target_mode : config->mode;
    controller = CreateController(controller_mode);
    if (controller == NULL)
    {
        return FB_PID_STATUS_BACKEND_ERROR;
    }
    if (config->mode == FB_PID_MODE_AUTO_TUNE)
    {
        auto_tune = ckCreateDtAutoTune();
        if (auto_tune == NULL)
        {
            (void)ckControllerFree(controller);
            return FB_PID_STATUS_BACKEND_ERROR;
        }
        error = ckAutoTuneSetEnumParam(auto_tune,
                                       DT_AUTO_TUNE_PARAM_CONTROL_ACTION,
                                       (int32_t)config->control_action);
        if (!CK_SUCCESS(error))
        {
            (void)ckAutoTuneFree(auto_tune);
            (void)ckControllerFree(controller);
            return FB_PID_STATUS_BACKEND_ERROR;
        }
    }
    error = SetControlAction(controller, controller_mode,
                             config->control_action);
    if (!CK_SUCCESS(error))
    {
        if (auto_tune != NULL) (void)ckAutoTuneFree(auto_tune);
        (void)ckControllerFree(controller);
        return FB_PID_STATUS_BACKEND_ERROR;
    }
    instance->controller = controller;
    instance->auto_tune = auto_tune;
    instance->initialized = true;
    instance->last_status = FB_PID_STATUS_OK;
    ClearOutput(&instance->output, FB_PID_STATUS_OK);
    return FB_PID_STATUS_OK;
}

void FbPid_Deinit(FbPid_t *instance)
{
    if (instance == NULL) return;
    if (AutoTune(instance) != NULL) (void)ckAutoTuneFree(AutoTune(instance));
    if (Controller(instance) != NULL) (void)ckControllerFree(Controller(instance));
    (void)memset(instance, 0, sizeof(*instance));
    instance->last_status = FB_PID_STATUS_NOT_INITIALIZED;
}

FbPidStatus_t FbPid_Reset(FbPid_t *instance)
{
    if (instance == NULL || !instance->initialized)
        return FB_PID_STATUS_NOT_INITIALIZED;
    instance->reset_input_previous = false;
    return ResetRuntime(instance) ? FB_PID_STATUS_RESET :
                                    FB_PID_STATUS_BACKEND_ERROR;
}

FbPidStatus_t FbPid_SetParameter(FbPid_t *instance,
                                 FbPidParameter_t parameter,
                                 int32_t value)
{
    CK_ERROR error;
    CK_CONTROLLER controller;
    CK_AUTO_TUNE autotune;
    CK_CONTROLLER_PARAM controller_param = 0;
    CK_AUTO_TUNE_PARAM autotune_param = 0;
    bool is_uint = false;
    bool is_bool = false;
    bool is_enum = false;

    if (instance == NULL || !instance->initialized)
        return FB_PID_STATUS_NOT_INITIALIZED;
    controller = Controller(instance);
    autotune = AutoTune(instance);
    switch (parameter)
    {
        case FB_PID_PARAMETER_ON_OFF_OUT1_TS: controller_param = DT_ON_OFF_PARAM_OUT1_TS; break;
        case FB_PID_PARAMETER_ON_OFF_OUT2_TS: controller_param = DT_ON_OFF_PARAM_OUT2_TS; break;
        case FB_PID_PARAMETER_DEADBAND:
            controller_param = ((instance->config.mode == FB_PID_MODE_AUTO_TUNE ?
                                 instance->config.auto_tune_target_mode :
                                 instance->config.mode) == FB_PID_MODE_ON_OFF) ?
                DT_ON_OFF_PARAM_DEADBAND : DT_PID_PARAM_DEADBAND; break;
        case FB_PID_PARAMETER_PB: controller_param = DT_PID_PARAM_PB; is_uint = true; break;
        case FB_PID_PARAMETER_TI: controller_param = DT_PID_PARAM_TI; is_uint = true; break;
        case FB_PID_PARAMETER_TD: controller_param = DT_PID_PARAM_TD; is_uint = true; break;
        case FB_PID_PARAMETER_COOL_PB: controller_param = DT_PID_PARAM_C_PB; is_uint = true; break;
        case FB_PID_PARAMETER_COOL_TI: controller_param = DT_PID_PARAM_C_TI; is_uint = true; break;
        case FB_PID_PARAMETER_COOL_TD: controller_param = DT_PID_PARAM_C_TD; is_uint = true; break;
        case FB_PID_PARAMETER_MV1_LIMIT_HIGH: controller_param = DT_PID_PARAM_MV1_LIMIT_HIGH; is_uint = true; break;
        case FB_PID_PARAMETER_MV1_LIMIT_LOW: controller_param = DT_PID_PARAM_MV1_LIMIT_LOW; is_uint = true; break;
        case FB_PID_PARAMETER_MV2_LIMIT_HIGH: controller_param = DT_PID_PARAM_MV2_LIMIT_HIGH; is_uint = true; break;
        case FB_PID_PARAMETER_MV2_LIMIT_LOW: controller_param = DT_PID_PARAM_MV2_LIMIT_LOW; is_uint = true; break;
        case FB_PID_PARAMETER_ALPHA: controller_param = DT_PID_PARAM_ALPHA; is_uint = true; break;
        case FB_PID_PARAMETER_BETA: controller_param = DT_PID_PARAM_BETA; is_uint = true; break;
        case FB_PID_PARAMETER_GAMMA: controller_param = DT_PID_PARAM_GAMMA; is_uint = true; break;
        case FB_PID_PARAMETER_IPRELOAD: controller_param = DT_PID_PARAM_IPRELOAD; is_uint = true; break;
        case FB_PID_PARAMETER_PDOF: controller_param = DT_PID_PARAM_PDOF; is_uint = true; break;
        case FB_PID_PARAMETER_PID_MODE: controller_param = DT_PID_PARAM_PID_MODE; is_enum = true; break;
        case FB_PID_PARAMETER_DISTURBANCE_COMPENSATION:
            controller_param = DT_PID_PARAM_DIS_COMP_SWITCH; is_bool = true; break;
        case FB_PID_PARAMETER_AUTOTUNE_SV_RATIO: autotune_param = DT_AUTO_TUNE_PARAM_SV_RATIO; is_uint = true; break;
        case FB_PID_PARAMETER_AUTOTUNE_PB_RATIO: autotune_param = DT_AUTO_TUNE_PARAM_PB_RATIO; is_uint = true; break;
        case FB_PID_PARAMETER_AUTOTUNE_TI_RATIO: autotune_param = DT_AUTO_TUNE_PARAM_TI_RATIO; is_uint = true; break;
        case FB_PID_PARAMETER_AUTOTUNE_MV1_LIMIT_HIGH: autotune_param = DT_AUTO_TUNE_PARAM_MV1_LIMIT_HIGH; is_uint = true; break;
        case FB_PID_PARAMETER_AUTOTUNE_MV2_LIMIT_HIGH: autotune_param = DT_AUTO_TUNE_PARAM_MV2_LIMIT_HIGH; is_uint = true; break;
        case FB_PID_PARAMETER_AUTOTUNE_DISTURBANCE_COMPENSATION:
            autotune_param = DT_AUTO_TUNE_PARAM_DIS_COMP_SWITCH; is_bool = true; break;
        default: return FB_PID_STATUS_INVALID_ARGUMENT;
    }
    if (is_uint && value < 0)
        return FB_PID_STATUS_INVALID_ARGUMENT;
    if (parameter >= FB_PID_PARAMETER_AUTOTUNE_SV_RATIO)
    {
        if (autotune == NULL) return FB_PID_STATUS_INVALID_CONFIG;
        if (is_bool) error = ckAutoTuneSetBoolParam(autotune, autotune_param, value != 0);
        else error = ckAutoTuneSetUIntParam(autotune, autotune_param, (uint32_t)value);
    }
    else if (is_uint) error = ckControllerSetUIntParam(controller, controller_param, (uint32_t)value);
    else if (is_bool) error = ckControllerSetBoolParam(controller, controller_param, value != 0);
    else if (is_enum) error = ckControllerSetEnumParam(controller, controller_param, value);
    else error = ckControllerSetIntParam(controller, controller_param, value);
    if (!CK_SUCCESS(error))
    {
        instance->backend_error = error;
        return FB_PID_STATUS_BACKEND_ERROR;
    }
    return FB_PID_STATUS_OK;
}

static CK_ERROR RunController(FbPid_t *instance, const FbPidInput_t *input)
{
    CK_ERROR error;
    uint32_t step = input->elapsed_ms / 10U;
    if ((input->elapsed_ms % 10U) >= 5U) ++step;
    if (input->elapsed_ms < 5U || step == 0U || step > (uint32_t)INT32_MAX)
        return CK_ERROR_INVALID_PARAMETER;
    error = ckControllerSetPV(Controller(instance), ToCklibTemperature(input->pv));
    if (CK_SUCCESS(error)) error = ckControllerSetSV(Controller(instance), ToCklibTemperature(input->sv));
    if (CK_SUCCESS(error)) error = ckControllerSetStepTime(Controller(instance), (int32_t)step);
    if (CK_SUCCESS(error))
    {
        CK_OUTPUT result;
        (void)memset(&result, 0, sizeof(result));
        error = ckControllerControl(Controller(instance), &result);
        if (CK_SUCCESS(error))
        {
            if (result.mv_count > 2U) return CK_ERROR_INVALID_PARAMETER;
            instance->output.mv[0] = result.mv[0];
            instance->output.mv[1] = result.mv[1];
            instance->output.mv_count = (uint8_t)result.mv_count;
            instance->output.valid = result.mv_count > 0U;
        }
    }
    return error;
}

FbPidStatus_t FbPid_Execute(FbPid_t *instance,
                            const FbPidInput_t *input,
                            FbPidOutput_t *output)
{
    CK_ERROR error;
    uint32_t delta;
    if (instance == NULL || input == NULL || output == NULL)
        return FB_PID_STATUS_INVALID_ARGUMENT;
    if (!instance->initialized)
    {
        ClearOutput(output, FB_PID_STATUS_NOT_INITIALIZED);
        return output->status;
    }
    instance->callback_count++;
    instance->last_elapsed_ms = input->elapsed_ms;
    if (input->reset && !instance->reset_input_previous)
    {
        instance->reset_input_previous = true;
        (void)ResetRuntime(instance);
        instance->active = input->enable && input->valid;
        instance->last_status = instance->output.status;
        *output = instance->output;
        return output->status;
    }
    instance->reset_input_previous = input->reset;
    if (!input->enable)
    {
        if (instance->active) (void)ResetRuntime(instance);
        instance->active = false;
        ClearOutput(&instance->output, FB_PID_STATUS_DISABLED);
        instance->last_status = FB_PID_STATUS_DISABLED;
        *output = instance->output;
        return output->status;
    }
    if (!input->valid)
    {
        if (instance->active) (void)ResetRuntime(instance);
        instance->active = false;
        ClearOutput(&instance->output, FB_PID_STATUS_INVALID_INPUT);
        instance->last_status = FB_PID_STATUS_INVALID_INPUT;
        *output = instance->output;
        return output->status;
    }
    if (instance->config.mode == FB_PID_MODE_AUTO_TUNE)
    {
        delta = (input->elapsed_ms >= instance->config.auto_tune_period_ms) ?
            input->elapsed_ms - instance->config.auto_tune_period_ms :
            instance->config.auto_tune_period_ms - input->elapsed_ms;
        if (delta > instance->config.auto_tune_period_tolerance_ms)
        {
            if (instance->active) (void)ResetRuntime(instance);
            instance->last_status = FB_PID_STATUS_INVALID_TIMING;
            ClearOutput(&instance->output, FB_PID_STATUS_INVALID_TIMING);
            *output = instance->output;
            return output->status;
        }
    }
    instance->active = true;
    if (instance->config.mode == FB_PID_MODE_AUTO_TUNE &&
        !instance->auto_tune_completed)
    {
        error = ckAutoTuneSetSV(AutoTune(instance), ToCklibTemperature(input->sv));
        if (CK_SUCCESS(error))
        {
            CK_OUTPUT tune_output;
            (void)memset(&tune_output, 0, sizeof(tune_output));
            error = ckAutoTuneCollectAndSignal(AutoTune(instance), ToCklibTemperature(input->pv),
                                               &tune_output);
            if (CK_SUCCESS(error) && tune_output.mv_count == 0U)
            {
                error = ckAutoTuneApplyParams(AutoTune(instance), Controller(instance));
                if (CK_SUCCESS(error))
                {
                    instance->auto_tune_completed = true;
                    error = RunController(instance, input);
                }
            }
            else if (CK_SUCCESS(error))
            {
                if (tune_output.mv_count > 2U) error = CK_ERROR_INVALID_PARAMETER;
                else
                {
                    instance->output.mv[0] = tune_output.mv[0];
                    instance->output.mv[1] = tune_output.mv[1];
                    instance->output.mv_count = (uint8_t)tune_output.mv_count;
                    instance->output.valid = tune_output.mv_count > 0U;
                }
            }
        }
    }
    else error = RunController(instance, input);
    if (!CK_SUCCESS(error))
    {
        instance->backend_error = error;
        instance->active = false;
        ClearOutput(&instance->output, FB_PID_STATUS_BACKEND_ERROR);
        instance->output.backend_error = error;
    }
    else
    {
        instance->backend_error = CK_NO_ERR;
        instance->output.status = FB_PID_STATUS_OK;
        instance->output.auto_tune_completed = instance->auto_tune_completed;
    }
    instance->last_status = instance->output.status;
    *output = instance->output;
    return output->status;
}
