#ifndef FB_SELECTOR_H
#define FB_SELECTOR_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FB_SELECTOR_INPUT_COUNT (4U)

typedef enum
{
    FB_SELECTOR_MODE_INDEX = 0,
    FB_SELECTOR_MODE_MINIMUM,
    FB_SELECTOR_MODE_MAXIMUM
} FbSelectorMode_t;

typedef struct
{
    FbSelectorMode_t mode;
    uint8_t input_count;
    uint8_t selected_index;
} FbSelectorConfig_t;

typedef struct
{
    float values[FB_SELECTOR_INPUT_COUNT];
    bool enable;
} FbSelectorInput_t;

typedef enum
{
    FB_SELECTOR_STATUS_OK = 0,
    FB_SELECTOR_STATUS_DISABLED,
    FB_SELECTOR_STATUS_INVALID_ARGUMENT,
    FB_SELECTOR_STATUS_INVALID_CONFIGURATION,
    FB_SELECTOR_STATUS_INVALID_INPUT
} FbSelectorStatus_t;

typedef struct
{
    float value;
    uint8_t selected_index;
    bool valid;
    FbSelectorStatus_t status;
} FbSelectorOutput_t;

void FbSelector_Initialize(FbSelectorConfig_t *config);
void FbSelector_Reset(FbSelectorOutput_t *output);

/* Equal minimum/maximum values resolve to the lowest input index. */
bool FbSelector_Run(const FbSelectorConfig_t *config,
                    const FbSelectorInput_t *input,
                    FbSelectorOutput_t *output);

#ifdef __cplusplus
}
#endif

#endif /* FB_SELECTOR_H */
