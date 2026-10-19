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
    FB_LOGIC_STATUS_INVALID_ARGUMENT
} FbLogicStatus_t;

typedef struct
{
    uint16_t value; /* 0 or 1 */
    FbLogicStatus_t status;
} FbLogicOutput_t;

/*
 * Evaluate four logical inputs. Only bit 0 of each input and inversion value
 * is used. The input arrays are never modified. When disabled, value is forced
 * to 0 and DISABLED is returned regardless of output_inverted. On enabled
 * calls, value is normalized to 0 or 1 and status reports success/error.
 */
FbLogicStatus_t FbLogic_Execute(
    bool enable,
    FbLogicOperator_t operator_type,
    const uint16_t inputs[FB_LOGIC_INPUT_COUNT],
    const uint16_t input_inversions[FB_LOGIC_INPUT_COUNT],
    bool output_inverted,
    FbLogicOutput_t *output);

#ifdef __cplusplus
}
#endif

#endif /* FB_LOGIC_H */
