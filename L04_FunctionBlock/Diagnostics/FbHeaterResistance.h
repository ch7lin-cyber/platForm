#ifndef FB_HEATER_RESISTANCE_H
#define FB_HEATER_RESISTANCE_H
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define FB_HEATER_RESISTANCE_MAX_MOHM UINT64_C(4294967295000)
typedef struct {
    uint32_t minimum_voltage_mv, minimum_current_ma; /* both >0 */
    uint32_t maximum_age_ms; /* 0 disables age limit, not skew check */
    uint32_t maximum_skew_ms;
    bool limits_enabled;
    uint64_t low_mohm, high_mohm;
    bool reference_enabled;
    uint64_t reference_mohm; /* 1..MAX when enabled */
} FbHeaterResistanceConfig;
typedef struct {
    uint32_t voltage_mv, current_ma; /* aligned RMS or DC magnitudes */
    uint32_t voltage_age_ms, current_age_ms; /* same 'now' and units */
    bool voltage_valid, current_valid, enable, energized;
} FbHeaterResistanceInput;
typedef enum { FB_HEATER_OK=0, FB_HEATER_DISABLED, FB_HEATER_INACTIVE,
    FB_HEATER_INVALID_ARGUMENT, FB_HEATER_INVALID_CONFIG,
    FB_HEATER_INVALID_INPUT, FB_HEATER_STALE, FB_HEATER_UNALIGNED,
    FB_HEATER_LOW_VOLTAGE, FB_HEATER_LOW_CURRENT } FbHeaterResistanceStatus;
typedef struct {
    uint64_t resistance_mohm;
    uint64_t apparent_power_mva; /* Vrms*Irms, not guaranteed real mW */
    int64_t reference_change_permille; /* signed R deviation / nominal */
    bool valid, below_limit, above_limit, reference_valid;
    FbHeaterResistanceStatus status;
} FbHeaterResistanceOutput;
/* Stateless single load. No Init/Reset, allocation or output actuation.
 * For resistive AC load V/I estimates R; for general load it is |Z|.
 * External RMS windows/scaling/common timestamp must be compatible.
 * Non-overlapping stable arguments required; all failures clear values. */
void FbHeaterResistance_DefaultConfig(FbHeaterResistanceConfig *config);
bool FbHeaterResistance_ValidateConfig(const FbHeaterResistanceConfig *config);
FbHeaterResistanceStatus FbHeaterResistance_Execute(
    const FbHeaterResistanceConfig *config, const FbHeaterResistanceInput *input,
    FbHeaterResistanceOutput *output);
#ifdef __cplusplus
}
#endif
#endif
