#include "FbLogic.h"

#include <stddef.h>

static uint16_t NormalizeInput(uint16_t input, uint16_t inverted)
{
    uint16_t value = input & 0x0001U;

    if ((inverted & 0x0001U) != 0U)
    {
        value ^= 0x0001U;
    }
    return value;
}

uint16_t FbLogic_Execute(
    FbLogicOperator_t operator_type,
    const uint16_t inputs[FB_LOGIC_INPUT_COUNT],
    const uint16_t input_inversions[FB_LOGIC_INPUT_COUNT],
    bool output_inverted)
{
    uint16_t values[FB_LOGIC_INPUT_COUNT];
    uint16_t result = 0U;
    uint8_t index;

    if ((inputs == NULL) || (input_inversions == NULL))
    {
        return 0U;
    }

    for (index = 0U; index < FB_LOGIC_INPUT_COUNT; index++)
    {
        values[index] = NormalizeInput(inputs[index],
                                       input_inversions[index]);
    }

    switch (operator_type)
    {
        case FB_LOGIC_OPERATOR_AND:
            result = values[0] & values[1] & values[2] & values[3];
            break;

        case FB_LOGIC_OPERATOR_OR:
            result = values[0] | values[1] | values[2] | values[3];
            break;

        case FB_LOGIC_OPERATOR_XOR:
            result = values[0] ^ values[1] ^ values[2] ^ values[3];
            break;

        case FB_LOGIC_OPERATOR_OFF:
        default:
            result = 0U;
            break;
    }

    if (output_inverted)
    {
        result ^= 0x0001U;
    }
    return result;
}
