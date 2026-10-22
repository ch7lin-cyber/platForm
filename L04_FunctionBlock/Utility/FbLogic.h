#ifndef FB_LOGIC_H
#define FB_LOGIC_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FB_LOGIC_INPUT_COUNT (4U)

typedef enum
{
    FB_LOGIC_OPERATOR_OFF = 0,
    FB_LOGIC_OPERATOR_AND,
    FB_LOGIC_OPERATOR_OR,
    FB_LOGIC_OPERATOR_XOR
} FbLogicOperator_t;

typedef enum
{
    FB_LOGIC_STATUS_OK = 0,
    FB_LOGIC_STATUS_DISABLED,
    FB_LOGIC_STATUS_INVALID_ARGUMENT,
    FB_LOGIC_STATUS_RESET,
    FB_LOGIC_STATUS_INVALID_CONFIGURATION
} FbLogicStatus_t;

typedef struct
{
    FbLogicOperator_t operator_type;
    uint16_t input_inversions[FB_LOGIC_INPUT_COUNT]; /* bit 0 only */
    bool output_inverted;
    uint32_t on_delay_ms; /* 0..UINT32_MAX; zero switches immediately */
    uint32_t off_delay_ms;
} FbLogicConfig_t;

typedef struct
{
    uint16_t inputs[FB_LOGIC_INPUT_COUNT]; /* bit 0 only */
    uint32_t elapsed_ms; /* actual interval since previous call, supplied by L05 */
    bool enable;
    bool reset; /* rising-edge command; reset scan publishes zero */
} FbLogicInput_t;

typedef struct
{
    uint16_t value; /* delayed output, 0 or 1 */
    FbLogicStatus_t status;
    uint16_t raw_value; /* after output inversion, before delay */
    bool delay_active;
    bool on_delay_active; /* waiting for 0 -> 1 */
    bool off_delay_active; /* waiting for 1 -> 0 */
    uint32_t delay_elapsed_ms;
} FbLogicOutput_t;

/* One caller-owned instance per independent block; no heap or hardware timer. */
typedef struct
{
    FbLogicConfig_t config;
    uint32_t delay_elapsed_ms;
    uint16_t value;
    bool delay_active;
    bool configured;
    bool reset_previous;
} FbLogic_t;

/* Initialize before first use (an all-zero instance is also valid). */
void FbLogic_Initialize(FbLogic_t *instance);

/* Default configuration: OFF, no inversions, zero ON/OFF delays. */
void FbLogic_DefaultConfig(FbLogicConfig_t *config);

/*
 * Input inversion -> Boolean operation -> output inversion -> ON/OFF delay.
 * First observation of a transition starts at zero elapsed time. Subsequent
 * calls accumulate elapsed_ms only while the target differs from the output.
 * Returning to the current output cancels the pending transition.
 * Disable, invalid arguments, RESET rising edge, and configuration changes
 * clear timing and output. Re-enable/recovery/config change start a fresh delay.
 * Disabled calls ignore config and force zero regardless of inversion/delay.
 * Operator OFF also forces zero and DISABLED, bypassing inversion/delays.
 * Only inversion bit 0 participates in computation and change detection.
 * State and configuration/input/output storage must not overlap. Serialize
 * calls on each instance. Configuration is copied for change detection.
 */
FbLogicStatus_t FbLogic_Execute(FbLogic_t *instance,
                                const FbLogicConfig_t *config,
                                const FbLogicInput_t *input,
                                FbLogicOutput_t *output);

#ifdef __cplusplus
}
#endif

#endif /* FB_LOGIC_H */
