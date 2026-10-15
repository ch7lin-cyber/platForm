#ifndef FB_WEIGHTED_SUM_H
#define FB_WEIGHTED_SUM_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FB_WEIGHTED_SUM_INPUT_COUNT (4U)

typedef struct
{
    uint8_t input_count;
} FbWeightedSumConfig_t;

typedef struct
{
    float values[FB_WEIGHTED_SUM_INPUT_COUNT];
    float weights[FB_WEIGHTED_SUM_INPUT_COUNT];
    bool enable;
} FbWeightedSumInput_t;

typedef enum
{
    FB_WEIGHTED_SUM_STATUS_OK = 0,
    FB_WEIGHTED_SUM_STATUS_DISABLED,
    FB_WEIGHTED_SUM_STATUS_INVALID_ARGUMENT,
    FB_WEIGHTED_SUM_STATUS_INVALID_CONFIGURATION,
    FB_WEIGHTED_SUM_STATUS_INVALID_INPUT,
    FB_WEIGHTED_SUM_STATUS_NONFINITE_RESULT
} FbWeightedSumStatus_t;

typedef struct
{
    float sum;
    bool valid;
    FbWeightedSumStatus_t status;
} FbWeightedSumOutput_t;

void FbWeightedSum_Initialize(FbWeightedSumConfig_t *config);
void FbWeightedSum_Reset(FbWeightedSumOutput_t *output);

/* Calculates sum(values[i] * weights[i]) for i < config->input_count. */
bool FbWeightedSum_Run(const FbWeightedSumConfig_t *config,
                       const FbWeightedSumInput_t *input,
                       FbWeightedSumOutput_t *output);

#ifdef __cplusplus
}
#endif

#endif /* FB_WEIGHTED_SUM_H */
