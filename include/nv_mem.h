#ifndef NV_MEM_H_
#define NV_MEM_H_

#include <stddef.h>

#ifdef NDEBUG

// Allocate a new chunck of memory.
// On failure the program is aborted.
void *memAlloc(size_t objectCount, size_t objectSize);
// Allocate a new chunck of memory given the size in bytes.
// On failure the program is aborted.
void *memAllocBytes(size_t byteCount);
// Allocate a new chunck of memory that is zeroed.
// On failure the program is aborted.
void *memAllocZeroed(size_t objectCount, size_t objectSize);
// Allocate a new chunck of memory that is zeroed.
// On failure the program is aborted.
void *memAllocZeroedBytes(size_t byteCount);

// Increase the size of a memory block.
// On failure the program is aborted.
void *memExpand(void *block, size_t newObjectCount, size_t objectSize);
// Increase the size of a memory block given the new size in bytes.
// On failure the program is aborted.
void *memExpandBytes(void *block, size_t newByteCount);

// Decrease the size of a memory block.
// If the block cannot be shrunk the block itself is returned.
void *memShrink(void *block, size_t newObjectCount, size_t objectSize);
// Decrease the size of a memory block given the new size in bytes.
// If the block cannot be shrunk the block itself is returned.
void *memShrinkBytes(void *block, size_t newByteCount);

// Change the state of `block` depending on `objectCount`.
// If `block == NULL` new memory will be allocated.
// If `block != NULL` and `objectCount == 0` the block will be free'd.
// Otherwise the block is reallocated.
// On failure the program is aborted.
void *memChange(void *block, size_t objectCount, size_t objectSize);

// Change the state of `block` depending on `byteCount`.
// If `block == NULL` new memory will be allocated.
// If `block != NULL` and `byteCount == 0` the block will be free'd.
// Otherwise the block is reallocated.
// On failure the program is aborted.
void *memChangeBytes(void *block, size_t byteCount);

// Free a block of memory. Do nothing if `block == NULL`
void memFree(void *block);

#else

#define memAlloc(objectCount, objectSize)                                      \
    memAlloc_(objectCount, objectSize, __LINE__, __FILE__)

#define memAllocBytes(byteCount)                                               \
    memAllocBytes_(byteCount, __LINE__, __FILE__)

#define memAllocZeroed(objectCount, objectSize)                                \
    memAllocZeroed_(objectCount, objectSize, __LINE__, __FILE__)

#define memExpand(block, newObjectCount, objectSize)                           \
    memExpand_(block, newObjectCount, objectSize, __LINE__, __FILE__)

#define memExpandBytes(block, newByteCount)                                    \
    memExpandBytes_(block, newByteCount, __LINE__, __FILE__)

#define memShrink(block, newObjectCount, objectSize)                           \
    memShrink_(block, newObjectCount, objectSize, __LINE__, __FILE__)

#define memShrinkBytes(block, newByteCount)                                    \
    memShrinkBytes_(block, newByteCount, __LINE__, __FILE__)

#define memChange(block, objectCount, objectSize)                              \
    memChange_(block, objectCount, objectSize, __LINE__, __FILE__)

#define memChangeBytes(block, byteCount)                                       \
    memChangeBytes_(block, byteCount, __LINE__, __FILE__)

#define memFree(block)                                                         \
    memFree_(block, __LINE__, __FILE__)

// Internal function for memory tracking

#include <stdint.h>

void *memAlloc_(
    size_t objectCount,
    size_t objectSize,
    uint32_t line,
    const char *file
);
void *memAllocBytes_(size_t byteCount, uint32_t line, const char *file);
void *memAllocZeroed_(
    size_t objectCount,
    size_t objectSize,
    uint32_t line,
    const char *file
);
void *memAllocZeroedBytes_(size_t byteCount, uint32_t line, const char *file);
void *memExpand_(
    void *block,
    size_t newObjectCount,
    size_t objectSize,
    uint32_t line,
    const char *file
);
void *memExpandBytes_(
    void *block,
    size_t newByteCount,
    uint32_t line,
    const char *file
);
void *memShrink_(
    void *block,
    size_t newObjectCount,
    size_t objectSize,
    uint32_t line,
    const char *file
);
void *memShrinkBytes_(
    void *block,
    size_t newByteCount,
    uint32_t line,
    const char *file
);
void *memChange_(
    void *block,
    size_t objectCount,
    size_t objectSize,
    uint32_t line,
    const char *file
);
void *memChangeBytes_(
    void *block,
    size_t byteCount,
    uint32_t line,
    const char *file
);
void memFree_(void *block, uint32_t line, const char *file);

#endif // !NDEBUG

#endif // !NV_MEM_H_
