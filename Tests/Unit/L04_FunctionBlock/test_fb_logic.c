#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "FbLogic.h"

static void TestOperators(void)
{
    const uint16_t all_true[FB_LOGIC_INPUT_COUNT] = {1U, 1U, 1U, 1U};
    const uint16_t mixed[FB_LOGIC_INPUT_COUNT] = {1U, 0U, 1U, 0U};
    const uint16_t odd[FB_LOGIC_INPUT_COUNT] = {1U, 0U, 0U, 0U};
    const uint16_t no_inversion[FB_LOGIC_INPUT_COUNT] = {0U, 0U, 0U, 0U};

    assert(FbLogic_Execute(FB_LOGIC_OPERATOR_OFF, all_true,
                           no_inversion, false) == 0U);
    assert(FbLogic_Execute(FB_LOGIC_OPERATOR_AND, all_true,
                           no_inversion, false) == 1U);
    assert(FbLogic_Execute(FB_LOGIC_OPERATOR_AND, mixed,
                           no_inversion, false) == 0U);
    assert(FbLogic_Execute(FB_LOGIC_OPERATOR_OR, mixed,
                           no_inversion, false) == 1U);
    assert(FbLogic_Execute(FB_LOGIC_OPERATOR_XOR, mixed,
                           no_inversion, false) == 0U);
    assert(FbLogic_Execute(FB_LOGIC_OPERATOR_XOR, odd,
                           no_inversion, false) == 1U);
}

static void TestInputAndOutputInversion(void)
{
    const uint16_t inputs[FB_LOGIC_INPUT_COUNT] = {1U, 0U, 1U, 0U};
    const uint16_t inversions[FB_LOGIC_INPUT_COUNT] = {0U, 1U, 0U, 1U};

    assert(FbLogic_Execute(FB_LOGIC_OPERATOR_AND, inputs,
                           inversions, false) == 1U);
    assert(FbLogic_Execute(FB_LOGIC_OPERATOR_AND, inputs,
                           inversions, true) == 0U);
}

static void TestOnlyBitZeroIsUsedAndInputsRemainUnchanged(void)
{
    uint16_t inputs[FB_LOGIC_INPUT_COUNT] =
        {0xFFFFU, 0x0002U, 0x0101U, 0x8000U};
    uint16_t inversions[FB_LOGIC_INPUT_COUNT] =
        {0x0002U, 0xFFFFU, 0x0000U, 0x0001U};
    uint16_t original_inputs[FB_LOGIC_INPUT_COUNT];
    uint16_t original_inversions[FB_LOGIC_INPUT_COUNT];

    (void)memcpy(original_inputs, inputs, sizeof(inputs));
    (void)memcpy(original_inversions, inversions, sizeof(inversions));

    assert(FbLogic_Execute(FB_LOGIC_OPERATOR_AND, inputs,
                           inversions, false) == 1U);
    assert(memcmp(inputs, original_inputs, sizeof(inputs)) == 0);
    assert(memcmp(inversions, original_inversions, sizeof(inversions)) == 0);
}

static void TestInvalidArguments(void)
{
    const uint16_t values[FB_LOGIC_INPUT_COUNT] = {1U, 1U, 1U, 1U};

    assert(FbLogic_Execute((FbLogicOperator_t)99, values,
                           values, false) == 0U);
    assert(FbLogic_Execute(FB_LOGIC_OPERATOR_AND, NULL,
                           values, false) == 0U);
    assert(FbLogic_Execute(FB_LOGIC_OPERATOR_AND, values,
                           NULL, false) == 0U);
}

int main(void)
{
    TestOperators();
    TestInputAndOutputInversion();
    TestOnlyBitZeroIsUsedAndInputsRemainUnchanged();
    TestInvalidArguments();
    return 0;
}
