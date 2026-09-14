#include <stdbool.h>
#include <stdio.h>
#include "nv_pool.h"
#include "nv_utils.h"
#include "clib_mem.h"

Pool poolMake(uint16_t blockSize) {
    return (Pool) {
        .pages = NULL,
        .freePages = NULL,
        .pageSize = blockSize * 2048,
        .blockSize = blockSize
    };
}

Pool poolMakeEx(uint16_t blockSize, uint32_t pageSize) {
    return (Pool) {
        .pages = NULL,
        .freePages = NULL,
        .pageSize = pageSize - pageSize % blockSize,
        .blockSize = blockSize
    };
}

PoolPage *getPage(Pool *p) {
    PoolPage *page;

    if (p->freePages != NULL) {
        page = p->freePages;
        p->freePages = page->next;
        return page;
    }

    size_t blockCount = p->pageSize / p->blockSize;

    page = memAllocZeroedBytes(
        sizeof(*page)
        + p->pageSize
        + sizeof(void *) * blockCount
    );

    page->freeList = (void **)((uint8_t *)(page + 1) + p->pageSize);
    page->freeCount = blockCount;
    for (size_t i = 0; i < blockCount; i++) {
        page->freeList[i] = page->data + (p->blockSize * i);
    }
    page->next = NULL;
    return page;
}

void *poolAlloc(Pool *p) {
    PoolPage *page;
    if (p->pages == NULL || p->pages->freeCount == 0) {
        page = getPage(p);
        page->next = p->pages;
        p->pages = page;
    } else {
        page = p->pages;
    }
    return page->freeList[--page->freeCount];
}

bool tryPageFree(Pool *p, PoolPage *page, void *block) {
    uint8_t *dataPtr = (uint8_t *)(page + 1);
    uint8_t *blockPtr = (uint8_t *)block;
    if (blockPtr < dataPtr || blockPtr >= dataPtr + p->pageSize) {
        return false;
    }
#ifdef NV_DEBUG
    nvAssert(
        (blockPtr - dataPtr) % p->blockSize == 0,
        "misaligned block in pool when freeing"
    );

    for (size_t i = 0; i < page->freeCount; i++) {
        if (page->freeList[i] == block) {
            nvAssert(false, "block double free in pool");
            return true;
        }
    }
#endif
    page->freeList[page->freeCount++] = block;
    return true;
}

void poolFree(Pool *p, void *block) {
    if (p->pages != NULL) {
        PoolPage *page = p->pages;
        // First try to delete from the currently allocating page.
        if (tryPageFree(p, page, block)) return;
        PoolPage *prev;
        // Then try to delete from all the full pages, if successfull move the
        // affected page to the free pages list.
        for (;;) {
            prev = page;
            page = prev->next;
            if (page == NULL) break;
            if (!tryPageFree(p, page, block)) continue;

            prev->next = page->next;
            page->next = p->freePages;
            p->freePages = page;
            return;
        }
    }
    // Lastly try to free from the partially full pages
    if (p->freePages != NULL) {
        PoolPage *page = p->freePages;
        while (page) {
            if (tryPageFree(p, page, block)) break;
        }
    }
    nvAssert(false, "attempted to free a non-existent block from a pool");
}

void poolDestroy(Pool *p) {
    PoolPage *page = p->pages;
    while (page) {
        PoolPage *curr = page;
        page = page->next;
        memFree(curr);
    }
    page = p->freePages;
    while (page) {
        PoolPage *curr = page;
        page = page->next;
        memFree(curr);
    }
    p->pages = NULL;
    p->freePages = NULL;
}
