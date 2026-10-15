#ifndef FB_DIVIDE_H
#define FB_DIVIDE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    /* A denominator whose absolute value is <= this limit is rejected. */
    float denominator_epsilon;
} FbDivideConfig_t;

typedef struct
{
    float numerator;
    float denominator;
    bool enable;
} FbDivideInput_t;

typedef enum
{
    FB_DIVIDE_STATUS_OK = 0,
    FB_DIVIDE_STATUS_DISABLED,
    FB_DIVIDE_STATUS_INVALID_ARGUMENT,
    FB_DIVIDE_STATUS_INVALID_CONFIGURATION,
    FB_DIVIDE_STATUS_INVALID_INPUT,
    FB_DIVIDE_STATUS_DIVIDE_BY_ZERO,
    FB_DIVIDE_STATUS_NONFINITE_RESULT
} FbDivideStatus_t;

typedef struct
{
    float quotient;
    bool valid;
    FbDivideStatus_t status;
} FbDivideOutput_t;

/* Set deterministic defaults: exact zero is the default divisor limit. */
void FbDivide_Initialize(FbDivideConfig_t *config);

/* Clear the published output to a disabled/invalid state. */
void FbDivide_Reset(FbDivideOutput_t *output);

/* Execute one stateless division; returns true only for a valid quotient. */
bool FbDivide_Run(const FbDivideConfig_t *config,
                  const FbDivideInput_t *input,
                  FbDivideOutput_t *output);

#ifdef __cplusplus
}
#endif

#endif /* FB_DIVIDE_H */
