// Pool allocator.

#ifndef NV_POOL_H_
#define NV_POOL_H_

#include <stdint.h>
#include <stddef.h>
#include "nv_array.h"

typedef struct PoolPage {
    struct PoolPage *next;
    void **freeList;
    uint64_t freeCount; // Force 8-byte alignment, even if size_t would suffice
    uint8_t data[];
} PoolPage;

typedef Arr(PoolPage *) PoolPages;

typedef struct Pool {
    PoolPage *pages;
    PoolPage *freePages;
    size_t pageSize;
    uint16_t blockSize;
} Pool;

// Make a pool with the default page size (2048 blocks).
Pool poolMake(uint16_t blockSize);
// Make a pool with a custom page size.
Pool poolMakeEx(uint16_t blockSize, uint32_t pageSize);

// Allocate one block from the pool (of size `p->blockSize`).
void *poolAlloc(Pool *p);
// Free one block from the pool.
void poolFree(Pool *p, void *block);

// Free all memory related to the pool.
void poolDestroy(Pool *p);

#endif // !NV_POOL_H_
