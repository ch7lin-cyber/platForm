#ifndef FB_POWER_VOLTAGE_COMPENSATION_H
#define FB_POWER_VOLTAGE_COMPENSATION_H
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum { FB_VOLTAGE_BYPASS=0, FB_VOLTAGE_SQUARED } FbVoltageCompensationMode;
typedef struct {
    FbVoltageCompensationMode mode;
    uint32_t nominal_mv, minimum_voltage_mv;
    uint32_t maximum_age_ms; /* 0 disables age rejection */
    uint32_t minimum_gain_permille, maximum_gain_permille; /* 1..10000 */
    uint16_t maximum_mv; /* 0..1000 in 0.1%; no nonzero lower clamp */
} FbPowerVoltageCompensationConfig;
typedef struct {
    int32_t mv; /* one nonnegative actuator demand, 0..1000 in 0.1% */
    uint32_t voltage_mv, voltage_age_ms;
    bool mv_valid, voltage_valid, enable;
} FbPowerVoltageCompensationInput;
typedef enum { FB_VOLTAGE_OK=0, FB_VOLTAGE_DISABLED, FB_VOLTAGE_INVALID_ARGUMENT,
    FB_VOLTAGE_INVALID_CONFIG, FB_VOLTAGE_INVALID_INPUT,
    FB_VOLTAGE_STALE, FB_VOLTAGE_TOO_LOW } FbVoltageCompensationStatus;
typedef struct {
    int32_t mv;
    double raw_gain, applied_gain, requested_mv;
    bool valid, bypassed, gain_limited, output_limited, at_limit;
    FbVoltageCompensationStatus status;
} FbPowerVoltageCompensationOutput;
/* Stateless single resistive actuator candidate. No heap, Init/Reset,
 * hardware or Safety policy. V^2 law assumes compatible full-on voltage and
 * linear power-fraction MV. Do not feed load-side PWM-averaged voltage.
 * All argument storage must be stable/non-overlapping for the call. */
void FbPowerVoltageCompensation_DefaultConfig(FbPowerVoltageCompensationConfig *config);
bool FbPowerVoltageCompensation_ValidateConfig(const FbPowerVoltageCompensationConfig *config);
FbVoltageCompensationStatus FbPowerVoltageCompensation_Execute(
    const FbPowerVoltageCompensationConfig *config,
    const FbPowerVoltageCompensationInput *input,
    FbPowerVoltageCompensationOutput *output);
#ifdef __cplusplus
}
#endif
#endif
