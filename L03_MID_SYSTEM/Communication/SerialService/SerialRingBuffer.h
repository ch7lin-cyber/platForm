#ifndef SERIAL_RING_BUFFER_H
#define SERIAL_RING_BUFFER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint8_t *storage;
    size_t capacity;
    volatile size_t head;
    volatile size_t tail;
} SerialRingBuffer_t;

bool SerialRingBuffer_Initialize(SerialRingBuffer_t *buffer,
                                 uint8_t *storage,
                                 size_t capacity);
void SerialRingBuffer_Clear(SerialRingBuffer_t *buffer);
size_t SerialRingBuffer_Count(const SerialRingBuffer_t *buffer);
size_t SerialRingBuffer_Free(const SerialRingBuffer_t *buffer);
bool SerialRingBuffer_PushFromIsr(SerialRingBuffer_t *buffer, uint8_t value);
size_t SerialRingBuffer_PushBlockFromIsr(SerialRingBuffer_t *buffer,
                                         const uint8_t *data,
                                         size_t length);
bool SerialRingBuffer_Pop(SerialRingBuffer_t *buffer, uint8_t *value);
size_t SerialRingBuffer_Read(SerialRingBuffer_t *buffer,
                             uint8_t *data,
                             size_t length);

#ifdef __cplusplus
}
#endif

#endif
