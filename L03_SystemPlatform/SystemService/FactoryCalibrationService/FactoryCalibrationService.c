#include "FactoryCalibrationService.h"

#include <stddef.h>
#include <string.h>

static HalAdcFactoryCalibration_t
    g_calibration[FACTORY_CALIBRATION_INPUT_COUNT]
                 [FACTORY_CALIBRATION_PROFILE_COUNT];
static int32_t g_live_uv[FACTORY_CALIBRATION_INPUT_COUNT];
static bool g_live_valid[FACTORY_CALIBRATION_INPUT_COUNT];
static FactoryCalibrationSnapshot_t g_snapshot;
static uint16_t g_unlock_key1;
static uint16_t g_unlock_key2;

static bool IsUnlocked(void)
{
    return (g_unlock_key1 == FACTORY_CALIBRATION_UNLOCK_KEY) &&
           (g_unlock_key2 == FACTORY_CALIBRATION_UNLOCK_KEY);
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

    (void)memset(g_live_valid, 0, sizeof(g_live_valid));
    (void)memset(&g_snapshot, 0, sizeof(g_snapshot));
    g_unlock_key1 = 0U;
    g_unlock_key2 = 0U;
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

void FactoryCalibrationService_SetUnlockKey1(uint16_t key)
{
    g_unlock_key1 = key;
    g_snapshot.state = IsUnlocked() ? FACTORY_CAL_STATE_READY :
                                      FACTORY_CAL_STATE_LOCKED;
}

void FactoryCalibrationService_SetUnlockKey2(uint16_t key)
{
    g_unlock_key2 = key;
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
        (profile >= FACTORY_CALIBRATION_PROFILE_COUNT))
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

    if (g_snapshot.state != FACTORY_CAL_STATE_ZERO_CAPTURED)
    {
        return Fail(FACTORY_CAL_ERROR_SEQUENCE);
    }
    if (!g_snapshot.live_valid)
    {
        return Fail(FACTORY_CAL_ERROR_NO_LIVE_SAMPLE);
    }
    span = (int64_t)g_snapshot.live_uv - g_snapshot.pending_zero_uv;
    if ((span > -1000LL) && (span < 1000LL))
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
    g_calibration[g_snapshot.input][g_snapshot.profile] = pending;
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
        (profile >= FACTORY_CALIBRATION_PROFILE_COUNT) ||
        (calibration == NULL))
    {
        return false;
    }
    *calibration = g_calibration[input][profile];
    return true;
}

void FactoryCalibrationService_GetSnapshot(
    FactoryCalibrationSnapshot_t *snapshot)
{
    if (snapshot != NULL)
    {
        *snapshot = g_snapshot;
    }
}
