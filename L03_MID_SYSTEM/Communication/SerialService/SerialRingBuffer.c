#include "SerialRingBuffer.h"

#include <stddef.h>

static size_t SerialRingBuffer_Next(const SerialRingBuffer_t *buffer,
                                    size_t index)
{
    index++;
    return (index >= buffer->capacity) ? 0U : index;
}

bool SerialRingBuffer_Initialize(SerialRingBuffer_t *buffer,
                                 uint8_t *storage,
                                 size_t capacity)
{
    if ((buffer == NULL) || (storage == NULL) || (capacity < 2U))
    {
        return false;
    }
    buffer->storage = storage;
    buffer->capacity = capacity;
    buffer->head = 0U;
    buffer->tail = 0U;
    return true;
}

void SerialRingBuffer_Clear(SerialRingBuffer_t *buffer)
{
    if (buffer != NULL)
    {
        buffer->tail = buffer->head;
    }
}

size_t SerialRingBuffer_Count(const SerialRingBuffer_t *buffer)
{
    size_t head;
    size_t tail;

    if ((buffer == NULL) || (buffer->storage == NULL))
    {
        return 0U;
    }
    head = buffer->head;
    tail = buffer->tail;
    return (head >= tail) ? (head - tail) :
                            (buffer->capacity - tail + head);
}

size_t SerialRingBuffer_Free(const SerialRingBuffer_t *buffer)
{
    if ((buffer == NULL) || (buffer->capacity < 2U))
    {
        return 0U;
    }
    return (buffer->capacity - 1U) - SerialRingBuffer_Count(buffer);
}

bool SerialRingBuffer_PushFromIsr(SerialRingBuffer_t *buffer, uint8_t value)
{
    size_t next;

    if ((buffer == NULL) || (buffer->storage == NULL))
    {
        return false;
    }
    next = SerialRingBuffer_Next(buffer, buffer->head);
    if (next == buffer->tail)
    {
        return false;
    }
    buffer->storage[buffer->head] = value;
    buffer->head = next;
    return true;
}

size_t SerialRingBuffer_PushBlockFromIsr(SerialRingBuffer_t *buffer,
                                         const uint8_t *data,
                                         size_t length)
{
    size_t written = 0U;

    if (data == NULL)
    {
        return 0U;
    }
    while ((written < length) &&
           SerialRingBuffer_PushFromIsr(buffer, data[written]))
    {
        written++;
    }
    return written;
}

bool SerialRingBuffer_Pop(SerialRingBuffer_t *buffer, uint8_t *value)
{
    if ((buffer == NULL) || (value == NULL) || (buffer->storage == NULL) ||
        (buffer->tail == buffer->head))
    {
        return false;
    }
    *value = buffer->storage[buffer->tail];
    buffer->tail = SerialRingBuffer_Next(buffer, buffer->tail);
    return true;
}

size_t SerialRingBuffer_Read(SerialRingBuffer_t *buffer,
                             uint8_t *data,
                             size_t length)
{
    size_t read_count = 0U;

    if (data == NULL)
    {
        return 0U;
    }
    while ((read_count < length) &&
           SerialRingBuffer_Pop(buffer, &data[read_count]))
    {
        read_count++;
    }
    return read_count;
}
