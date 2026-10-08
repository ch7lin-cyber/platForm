#include "FactoryCalibrationService.h"

#include <stddef.h>
#include <string.h>

#include "FactoryModeService.h"

static HalAdcFactoryCalibration_t
    g_calibration[FACTORY_CALIBRATION_INPUT_COUNT]
                 [FACTORY_CALIBRATION_PROFILE_COUNT];
static int32_t g_live_uv[FACTORY_CALIBRATION_INPUT_COUNT];
static bool g_live_valid[FACTORY_CALIBRATION_INPUT_COUNT];
/* One bit per profile avoids adding padding to every calibration record. */
static uint16_t g_calibrated_profiles[FACTORY_CALIBRATION_INPUT_COUNT];
static FactoryCalibrationSnapshot_t g_snapshot;
static const FactoryCalibrationTargets_t *g_targets;
static FactoryCalibrationSave_t g_save;
static bool IsUnlocked(void)
{
    return FactoryModeService_IsActive();
}

static bool Fail(FactoryCalibrationError_t error)
{
    g_snapshot.error = error;
    g_snapshot.state = FACTORY_CAL_STATE_ERROR;
    return false;
}

void FactoryCalibrationService_Initialize(void)
{
    uint8_t input;
    uint8_t profile;

    g_targets = NULL;
    g_save = NULL;
    (void)memset(g_live_valid, 0, sizeof(g_live_valid));
    (void)memset(g_calibrated_profiles, 0, sizeof(g_calibrated_profiles));
    (void)memset(&g_snapshot, 0, sizeof(g_snapshot));
    g_snapshot.state = FACTORY_CAL_STATE_LOCKED;
    for (input = 0U; input < FACTORY_CALIBRATION_INPUT_COUNT; input++)
    {
        for (profile = 0U; profile < FACTORY_CALIBRATION_PROFILE_COUNT;
             profile++)
        {
            g_calibration[input][profile].measured_zero_uv = 0L;
            g_calibration[input][profile].measured_span_uv = 30000L;
            g_calibration[input][profile].valid = true;
        }
    }
}

bool FactoryCalibrationService_GetTargets(FactoryCalibrationProfile_t profile,
                                         FactoryCalibrationTargets_t *targets)
{
    if (((unsigned int)profile >= FACTORY_CALIBRATION_PROFILE_COUNT) || (targets == NULL))
        return false;
    if (g_targets != NULL) *targets = g_targets[profile];
    else { targets->zero_uv = 0L; targets->span_uv = 30000L; }
    return true;
}

bool FactoryCalibrationService_SetTargets(const FactoryCalibrationTargets_t *targets,
                                         uint8_t count)
{
    uint8_t input, profile;
    if ((targets == NULL) || (count != FACTORY_CALIBRATION_PROFILE_COUNT)) return false;
    for (profile = 0U; profile < count; profile++)
    {
        int64_t span = (int64_t)targets[profile].span_uv - targets[profile].zero_uv;
        if ((span < 1000LL) || (span > 2147483647LL)) return false;
    }
    /* Target geometry cannot change underneath an already calibrated record. */
    for (input = 0U; input < FACTORY_CALIBRATION_INPUT_COUNT; input++)
        if (g_calibrated_profiles[input] != 0U) return false;
    g_targets = targets;
    for (input = 0U; input < FACTORY_CALIBRATION_INPUT_COUNT; input++)
    for (profile = 0U; profile < count; profile++)
    {
        g_calibration[input][profile].measured_zero_uv = targets[profile].zero_uv;
        g_calibration[input][profile].measured_span_uv = targets[profile].span_uv;
    }
    return true;
}

void FactoryCalibrationService_SetSaveCallback(FactoryCalibrationSave_t save)
{
    g_save = save;
}

bool FactoryCalibrationService_Restore(uint8_t input,
    FactoryCalibrationProfile_t profile, const HalAdcFactoryCalibration_t *calibration)
{
    if ((input >= FACTORY_CALIBRATION_INPUT_COUNT) ||
        ((unsigned int)profile >= FACTORY_CALIBRATION_PROFILE_COUNT) ||
        !HalAdcMeasurement_ValidateFactoryCalibration(calibration) ||
        (calibration->measured_span_uv <= calibration->measured_zero_uv)) return false;
    g_calibration[input][profile] = *calibration;
    g_calibrated_profiles[input] |= (uint16_t)(1U << (unsigned int)profile);
    return true;
}

bool FactoryCalibrationService_Convert(uint8_t input,
    FactoryCalibrationProfile_t profile, int32_t raw_uv, int32_t *calibrated_uv)
{
    HalAdcFactoryCalibration_t calibration;
    FactoryCalibrationTargets_t targets;
    return FactoryCalibrationService_GetCalibration(input, profile, &calibration) &&
           FactoryCalibrationService_GetTargets(profile, &targets) &&
           HalAdcMeasurement_ApplyFactoryCalibrationTargets(raw_uv, &calibration,
               targets.zero_uv, targets.span_uv, calibrated_uv);
}

void FactoryCalibrationService_SetUnlockKey1(uint16_t key)
{
    FactoryModeService_SetUnlockKey1(key);
    g_snapshot.state = IsUnlocked() ? FACTORY_CAL_STATE_READY :
                                      FACTORY_CAL_STATE_LOCKED;
}

void FactoryCalibrationService_SetUnlockKey2(uint16_t key)
{
    FactoryModeService_SetUnlockKey2(key);
    g_snapshot.state = IsUnlocked() ? FACTORY_CAL_STATE_READY :
                                      FACTORY_CAL_STATE_LOCKED;
}

bool FactoryCalibrationService_Select(uint8_t input,
                                      FactoryCalibrationProfile_t profile)
{
    if (!IsUnlocked())
    {
        return Fail(FACTORY_CAL_ERROR_LOCKED);
    }
    if ((input >= FACTORY_CALIBRATION_INPUT_COUNT) ||
        ((unsigned int)profile >= FACTORY_CALIBRATION_PROFILE_COUNT))
    {
        return Fail(FACTORY_CAL_ERROR_INVALID_ARGUMENT);
    }
    g_snapshot.input = input;
    g_snapshot.profile = profile;
    g_snapshot.live_uv = g_live_uv[input];
    g_snapshot.live_valid = g_live_valid[input];
    g_snapshot.pending_zero_uv = 0L;
    g_snapshot.pending_span_uv = 0L;
    g_snapshot.error = FACTORY_CAL_ERROR_NONE;
    g_snapshot.state = FACTORY_CAL_STATE_READY;
    return true;
}

void FactoryCalibrationService_UpdateLiveMicrovolts(uint8_t input,
                                                     int32_t microvolts)
{
    if (input < FACTORY_CALIBRATION_INPUT_COUNT)
    {
        g_live_uv[input] = microvolts;
        g_live_valid[input] = true;
        if (g_snapshot.input == input)
        {
            g_snapshot.live_uv = microvolts;
            g_snapshot.live_valid = true;
        }
    }
}

bool FactoryCalibrationService_CaptureZero(void)
{
    if (!IsUnlocked())
    {
        return Fail(FACTORY_CAL_ERROR_LOCKED);
    }
    if (!g_snapshot.live_valid)
    {
        return Fail(FACTORY_CAL_ERROR_NO_LIVE_SAMPLE);
    }
    g_snapshot.pending_zero_uv = g_snapshot.live_uv;
    g_snapshot.error = FACTORY_CAL_ERROR_NONE;
    g_snapshot.state = FACTORY_CAL_STATE_ZERO_CAPTURED;
    return true;
}

bool FactoryCalibrationService_CaptureSpan(void)
{
    int64_t span;

    if (!IsUnlocked()) return Fail(FACTORY_CAL_ERROR_LOCKED);
    if (g_snapshot.state != FACTORY_CAL_STATE_ZERO_CAPTURED)
    {
        return Fail(FACTORY_CAL_ERROR_SEQUENCE);
    }
    if (!g_snapshot.live_valid)
    {
        return Fail(FACTORY_CAL_ERROR_NO_LIVE_SAMPLE);
    }
    span = (int64_t)g_snapshot.live_uv - g_snapshot.pending_zero_uv;
    if (span < 1000LL)
    {
        return Fail(FACTORY_CAL_ERROR_SPAN_TOO_SMALL);
    }
    g_snapshot.pending_span_uv = g_snapshot.live_uv;
    g_snapshot.error = FACTORY_CAL_ERROR_NONE;
    g_snapshot.state = FACTORY_CAL_STATE_SPAN_CAPTURED;
    return true;
}

bool FactoryCalibrationService_Apply(void)
{
    HalAdcFactoryCalibration_t pending;

    if (g_snapshot.state != FACTORY_CAL_STATE_SPAN_CAPTURED)
    {
        return Fail(FACTORY_CAL_ERROR_SEQUENCE);
    }
    pending.measured_zero_uv = g_snapshot.pending_zero_uv;
    pending.measured_span_uv = g_snapshot.pending_span_uv;
    pending.valid = true;
    if (!HalAdcMeasurement_ValidateFactoryCalibration(&pending))
    {
        return Fail(FACTORY_CAL_ERROR_SPAN_TOO_SMALL);
    }
    if (!IsUnlocked()) return Fail(FACTORY_CAL_ERROR_LOCKED);
    if ((g_save != NULL) && !g_save(g_snapshot.input, g_snapshot.profile, &pending))
        return Fail(FACTORY_CAL_ERROR_STORAGE);
    g_calibration[g_snapshot.input][g_snapshot.profile] = pending;
    g_calibrated_profiles[g_snapshot.input] |=
        (uint16_t)(1U << (unsigned int)g_snapshot.profile);
    g_snapshot.revision++;
    if (g_snapshot.revision == 0U)
    {
        g_snapshot.revision = 1U;
    }
    g_snapshot.error = FACTORY_CAL_ERROR_NONE;
    g_snapshot.state = FACTORY_CAL_STATE_COMPLETE;
    return true;
}

void FactoryCalibrationService_Abort(void)
{
    g_snapshot.pending_zero_uv = 0L;
    g_snapshot.pending_span_uv = 0L;
    g_snapshot.error = FACTORY_CAL_ERROR_NONE;
    g_snapshot.state = IsUnlocked() ? FACTORY_CAL_STATE_READY :
                                      FACTORY_CAL_STATE_LOCKED;
}

bool FactoryCalibrationService_GetCalibration(
    uint8_t input, FactoryCalibrationProfile_t profile,
    HalAdcFactoryCalibration_t *calibration)
{
    if ((input >= FACTORY_CALIBRATION_INPUT_COUNT) ||
        ((unsigned int)profile >= FACTORY_CALIBRATION_PROFILE_COUNT) ||
        (calibration == NULL))
    {
        return false;
    }
    *calibration = g_calibration[input][profile];
    return true;
}

bool FactoryCalibrationService_IsCalibrated(
    uint8_t input, FactoryCalibrationProfile_t profile)
{
    return (input < FACTORY_CALIBRATION_INPUT_COUNT) &&
           ((unsigned int)profile < FACTORY_CALIBRATION_PROFILE_COUNT) &&
           ((g_calibrated_profiles[input] &
             (uint16_t)(1U << (unsigned int)profile)) != 0U);
}

void FactoryCalibrationService_GetSnapshot(
    FactoryCalibrationSnapshot_t *snapshot)
{
    if (snapshot != NULL)
    {
        *snapshot = g_snapshot;
    }
}
