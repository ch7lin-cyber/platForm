/* Included only by FbPid.c with FB_PID_TEST_BACKEND=1. No CKLIB dependency.
 * last_elapsed_ms/callback_count retain their public diagnostic meanings.
 * backend_error stores no test settings; controller/auto_tune remain NULL.
 * Test MV is stored in a dedicated field (build-wide header contract). */
static FbPidStatus_t TestClear(FbPid_t *p, FbPidStatus_t status)
{
    memset(&p->output, 0, sizeof(p->output));
    p->output.status=status; p->last_status=status; p->active=false;
    return status;
}
FbPidStatus_t FbPid_Init(FbPid_t *p, const FbPidConfig_t *c)
{
    if (!p || !c) return FB_PID_STATUS_INVALID_CONFIG;
    if (c->mode!=FB_PID_MODE_PID || c->control_action!=FB_PID_ACTION_HEAT ||
        c->auto_tune_target_mode!=FB_PID_MODE_PID)
        return FB_PID_STATUS_INVALID_CONFIG;
    memset(p,0,sizeof(*p)); p->config=*c; p->initialized=true;
    return TestClear(p,FB_PID_STATUS_OK);
}
void FbPid_Deinit(FbPid_t *p)
{
    if (p) { memset(p,0,sizeof(*p)); p->last_status=FB_PID_STATUS_NOT_INITIALIZED; }
}
FbPidStatus_t FbPid_Reset(FbPid_t *p)
{
    if (!p || !p->initialized) return FB_PID_STATUS_NOT_INITIALIZED;
    p->reset_input_previous=false;
    return TestClear(p,FB_PID_STATUS_RESET);
}
FbPidStatus_t FbPid_SetTestMv(FbPid_t *p,int32_t mv)
{
    if (!p || !p->initialized) return FB_PID_STATUS_NOT_INITIALIZED;
    if (mv<0 || mv>1000) return FB_PID_STATUS_INVALID_ARGUMENT;
    p->test_mv=mv; return FB_PID_STATUS_OK;
}
FbPidStatus_t FbPid_SetParameter(FbPid_t *p,FbPidParameter_t parameter,int32_t value)
{
    (void)parameter; (void)value;
    return (!p || !p->initialized) ? FB_PID_STATUS_NOT_INITIALIZED : FB_PID_STATUS_INVALID_CONFIG;
}
FbPidStatus_t FbPid_Execute(FbPid_t *p,const FbPidInput_t *in,FbPidOutput_t *out)
{
    FbPidStatus_t status;
    if (!p || !in || !out) return FB_PID_STATUS_INVALID_ARGUMENT;
    if (!p->initialized) {
        memset(out,0,sizeof(*out)); out->status=FB_PID_STATUS_NOT_INITIALIZED;
        return out->status;
    }
    ++p->callback_count; p->last_elapsed_ms=in->elapsed_ms;
    if (in->reset && !p->reset_input_previous) status=FB_PID_STATUS_RESET;
    else if (!in->enable) status=FB_PID_STATUS_DISABLED;
    else if (!in->valid) status=FB_PID_STATUS_INVALID_INPUT;
    else if (in->elapsed_ms<5U) status=FB_PID_STATUS_INVALID_TIMING;
    else status=FB_PID_STATUS_OK;
    p->reset_input_previous=in->reset;
    TestClear(p,status);
    if (status==FB_PID_STATUS_OK) {
        p->output.mv[0]=p->test_mv; p->output.mv_count=1;
        p->output.valid=true; p->active=true;
    }
    *out=p->output; return status;
}
