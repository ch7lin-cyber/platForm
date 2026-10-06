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

/*
 * Evaluate four logical inputs. Only bit 0 of each input and inversion value
 * is used. The input arrays are never modified. The return value is always
 * normalized to 0U or 1U.
 */
uint16_t FbLogic_Execute(
    FbLogicOperator_t operator_type,
    const uint16_t inputs[FB_LOGIC_INPUT_COUNT],
    const uint16_t input_inversions[FB_LOGIC_INPUT_COUNT],
    bool output_inverted);

#ifdef __cplusplus
}
#endif

#endif /* FB_LOGIC_H */
