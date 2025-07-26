#ifndef NV_MEM_H_
#define NV_MEM_H_

#include <stddef.h>

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

#endif // !NV_MEM_H_
