#ifndef FB_LINEARIZATION_H
#define FB_LINEARIZATION_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FB_LINEARIZATION_OK = 0,
    FB_LINEARIZATION_DISABLED,
    FB_LINEARIZATION_INVALID_ARGUMENT,
    FB_LINEARIZATION_INVALID_CONFIGURATION,
    FB_LINEARIZATION_INVALID_INPUT,
    FB_LINEARIZATION_OUT_OF_RANGE
} FbLinearizationStatus;

typedef enum {
    FB_LINEARIZATION_CLAMP = 0,
    FB_LINEARIZATION_ERROR
} FbLinearizationPolicy;

typedef enum {
    FB_LINEARIZATION_RANGE_UNKNOWN = 0,
    FB_LINEARIZATION_RANGE_BELOW,
    FB_LINEARIZATION_RANGE_WITHIN,
    FB_LINEARIZATION_RANGE_ABOVE
} FbLinearizationRange;

typedef struct {
    float x;
    float y;
} FbLinearizationPoint;

typedef struct {
    const FbLinearizationPoint *table;
    size_t count; /* >= 2; every x/y finite, x strictly increasing */
    FbLinearizationPolicy policy; /* CLAMP or ERROR; never extrapolate */
} FbLinearizationConfig;

typedef struct {
    float value; /* finite; same caller-defined units as table x */
    bool enable;
} FbLinearizationInput;

typedef struct {
    float value; /* table y units; zero whenever valid is false */
    FbLinearizationRange range; /* endpoints are WITHIN; UNKNOWN on failure */
    bool valid;
    FbLinearizationStatus status;
} FbLinearizationOutput;

/* Stateless: no Init/Reset, allocation, retained table pointer or mutable
 * shared state. Each enabled call validates the entire table before use.
 * Null arguments precede enable; disabled calls ignore all table/numeric
 * fields. All non-OK results clear value/valid/range to 0/false/UNKNOWN.
 * With a non-null output, output.status always equals the return value.
 * Exact nodes copy y directly. Interpolation uses a four-term double expansion
 * and clamps to the segment's y bounds before converting back to float.
 *
 * Caller owns config/input/output and, when enabled, at least count correctly
 * typed table elements. Non-null arguments must reference valid, non-overlapping storage
 * for the full call; table/config/input must not change during that call.
 * Calls may share immutable config/table/input with separate outputs; callers
 * must synchronize any mutation or shared output. No pointer validity or
 * storage extent can be inferred from count. Requires binary double with room
 * for exact float products and their sums (compile-time checked), round-to-nearest,
 * and no fast-math/flush-to-zero. Verified with IEEE binary32/binary64. See the module's
 * _UnitTest/TestResult/FbLinearization_HostTest.md for numeric limits. */
FbLinearizationStatus FbLinearization_Execute(
    const FbLinearizationConfig *config, const FbLinearizationInput *input,
    FbLinearizationOutput *output);

#ifdef __cplusplus
}
#endif

#endif /* FB_LINEARIZATION_H */
