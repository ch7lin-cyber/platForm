#ifndef FB_MOVING_AVERAGE_H
#define FB_MOVING_AVERAGE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FB_MOVING_AVERAGE_OK = 0,
    FB_MOVING_AVERAGE_DISABLED,
    FB_MOVING_AVERAGE_INVALID_ARGUMENT,
    FB_MOVING_AVERAGE_INVALID_INPUT
} FbMovingAverageStatus;

typedef struct {
    size_t window_size; /* 1..buffer_capacity */
} FbMovingAverageConfig;

typedef struct {
    double sample;
    int enable;        /* 0 clears history/output and reports DISABLED */
    int sample_valid; /* 0 skips this cycle without changing the window */
    int reset;        /* clears the window before processing this cycle */
} FbMovingAverageInput;

typedef struct {
    double *buffer;       /* caller-owned storage, at least buffer_capacity */
    size_t buffer_capacity;
    size_t next_index;
    size_t sample_count;
    long double sum;
    size_t active_window_size;
    int initialized;
} FbMovingAverageState;

typedef struct {
    double average;
    size_t sample_count;
    int valid; /* true once at least one sample has been accumulated */
} FbMovingAverageOutput;

/* Initialize a block instance. Storage is caller-owned and must outlive state. */
FbMovingAverageStatus FbMovingAverage_Init(
    FbMovingAverageState *state,
    double *buffer,
    size_t buffer_capacity);

/*
 * Execute one sample. A valid sample replaces the oldest sample when full.
 * A reset clears history first; if sample_valid is also true, that sample is
 * then the first sample of the new window. Invalid samples leave history intact.
 */
FbMovingAverageStatus FbMovingAverage_Execute(
    const FbMovingAverageConfig *config,
    const FbMovingAverageInput *input,
    FbMovingAverageState *state,
    FbMovingAverageOutput *output);

#ifdef __cplusplus
}
#endif

#endif /* FB_MOVING_AVERAGE_H */
