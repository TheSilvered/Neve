#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "nv_mem.h"

#ifdef NDEBUG

void *memAlloc(size_t objectSize, size_t objectCount) {
    void *block = malloc(objectSize * objectCount);
    if (block == NULL) {
        fprintf(stderr, "Out of memory.");
        abort();
    }
    return block;
}

void *memAllocBytes(size_t byteCount) {
    void *block = malloc(byteCount);
    if (block == NULL) {
        fprintf(stderr, "Out of memory.");
        abort();
    }
    return block;
}

void *memAllocZeroed(size_t objectCount, size_t objectSize) {
    void *block = calloc(objectCount, objectSize);
    if (block == NULL) {
        fprintf(stderr, "Out of memory.");
        abort();
    }
    return block;
}

void *memAllocZeroedBytes(size_t byteCount) {
    void *block = calloc(byteCount, 1);
    if (block == NULL) {
        fprintf(stderr, "Out of memory.");
        abort();
    }
    return block;
}

void *memExpand(void *block, size_t objectSize, size_t newCount) {
    assert(objectSize != 0);
    assert(newCount != 0);
    void *newBlock;
    if (block == NULL) {
        newBlock = malloc(objectSize * newCount);
    } else {
        newBlock = realloc(block, objectSize * newCount);
    }
    if (newBlock == NULL) {
        fprintf(stderr, "Out of memory.");
        abort();
    }
    return newBlock;
}

void *memExpandBytes(void *block, size_t newByteCount) {
    assert(newByteCount != 0);
    void *newBlock;
    if (block == NULL) {
        newBlock = malloc(newByteCount);
    } else {
        newBlock = realloc(block, newByteCount);
    }
    if (newBlock == NULL) {
        fprintf(stderr, "Out of memory.");
        abort();
    }
    return newBlock;
}

void *memShrink(void *block, size_t objectSize, size_t newObjectCount) {
    assert(block != NULL);
    assert(objectSize != 0);
    assert(newObjectCount != 0);
    void *newBlock = realloc(block, objectSize * newObjectCount);
    if (newBlock == NULL) {
        return block;
    }
    return newBlock;
}

void *memShrinkBytes(void *block, size_t newByteCount) {
    assert(block != NULL);
    assert(newByteCount != 0);
    void *newBlock = realloc(block, newByteCount);
    if (newBlock == NULL) {
        return block;
    }
    return newBlock;
}

void *memChange(void *block, size_t objectSize, size_t objectCount) {
    if (block == NULL) {
        return memAlloc(objectSize, objectCount);
    } else if (objectSize == 0 || objectCount == 0) {
        memFree(block);
        return NULL;
    } else {
        void *newBlock = realloc(block, objectSize * objectCount);
        if (newBlock == NULL) {
            printf("Out of memory.");
            abort();
        }
        return newBlock;
    }
}

void *memChangeBytes(void *block, size_t byteCount) {
    if (block == NULL) {
        return memAllocBytes(byteCount);
    } else if (byteCount == 0) {
        memFree(block);
        return NULL;
    } else {
        void *newBlock = realloc(block, byteCount);
        if (newBlock == NULL) {
            printf("Out of memory.");
            abort();
        }
        return newBlock;
    }
}

void memFree(void *block) {
    if (block != NULL) {
        free(block);
    }
}

#else

#include <string.h>
#include <stdbool.h>
#include <inttypes.h>

#define sentinelLen_ 4
#define garbageByte_ 0xc5

#if __STDC_VERSION__ >= 201112L
    #if __STDC_VERSION__ < 202311L
        #define thread_local _Thread_local
    #endif
#elif defined(_MSC_VER) && !defined(__clang__)
#define thread_local __declspec( thread )
#else
#define thread_local __thread
#endif // !thread_local

// Keep the headers in an AVL tree sorted by memory address

typedef struct MemHeader {
    struct MemHeader *left, *right;
    uint32_t height;
    uint32_t line;
    const char *file; // assume static storage for file names
    size_t blockSize;
    uint64_t sentinels[sentinelLen_]; // pseudo-random sentinel pattern repeated
                                      // at the end
} MemHeader;

thread_local MemHeader *memRoot = NULL;

static uint32_t max_(uint32_t a, uint32_t b) {
    return a > b ? a : b;
}

static uint32_t min_(uint32_t a, uint32_t b) {
    return a < b ? a : b;
}

static uint32_t height_(MemHeader *header) {
    return header ? header->height : 0;
}

static MemHeader *rotRight_(MemHeader *oldRoot) {
    MemHeader *newRoot = oldRoot->left;
    MemHeader *tmp = newRoot->right;

    newRoot->right = oldRoot;
    oldRoot->left = tmp;

    oldRoot->height = max_(height_(oldRoot->left), height_(oldRoot->right)) + 1;
    newRoot->height = max_(height_(newRoot->left), height_(newRoot->right)) + 1;

    return newRoot;
}

static MemHeader *rotLeft_(MemHeader *oldRoot) {
    MemHeader *newRoot = oldRoot->right;
    MemHeader *tmp = newRoot->left;

    newRoot->left = oldRoot;
    oldRoot->right = tmp;

    oldRoot->height = max_(height_(oldRoot->left), height_(oldRoot->right)) + 1;
    newRoot->height = max_(height_(newRoot->left), height_(newRoot->right)) + 1;

    return newRoot;
}

static MemHeader *rebalance_(MemHeader *root) {
    int32_t balance = (int32_t)height_(root->left)
                    - (int32_t)height_(root->right);

    if (balance > 1) {
        int32_t leftBalance = (int32_t)height_(root->left->left)
                            - (int32_t)height_(root->left->right);
        if (leftBalance < 0) {
            root->left = rotLeft_(root->left);
        }
        return rotRight_(root);
    } else if (balance < -1) {
        int32_t rightBalance = (int32_t)height_(root->right->left)
                             - (int32_t)height_(root->right->right);
        if (rightBalance > 0) {
            root->right = rotRight_(root->right);
        }
        return rotLeft_(root);
    } else {
        return root;
    }
}

static MemHeader *insertHeader_(MemHeader *root, MemHeader *header) {
    if (root == NULL) {
        return header;
    }

    assert(header != root);
    if ((uintptr_t)header < (uintptr_t)root) {
        root->left = insertHeader_(root->left, header);
    } else {
        root->right = insertHeader_(root->right, header);
    }
    root->height = 1 + max_(height_(root->left), height_(root->right));

    return rebalance_(root);
}

static bool hasHeader_(MemHeader *root, MemHeader *header) {
    if (root == NULL) {
        return false;
    } else if ((uintptr_t)root == (uintptr_t)header) {
        return true;
    } else if ((uintptr_t)header < (uintptr_t)root) {
        return hasHeader_(root->left, header);
    } else {
        return hasHeader_(root->right, header);
    }
}

static MemHeader *minHeader_(MemHeader *root) {
    while (root->left) {
        root = root->left;
    }
    return root;
}

static MemHeader *removeHeader_(MemHeader *root, MemHeader *header) {
    if ((uintptr_t)header < (uintptr_t)root) {
        root->left = removeHeader_(root->left, header);
    } else if ((uintptr_t)header > (uintptr_t)root) {
        root->right = removeHeader_(root->right, header);
    } else {
        if (!root->left) {
            return root->right;
        } else if (!root->right) {
            return root->left;
        }

        // Find smallest value of the right subtree and use it in place of root
        MemHeader *newRoot = minHeader_(root->right);
        newRoot->right = removeHeader_(root->right, newRoot);
        newRoot->left = root->left;
        root = newRoot;
    }
    root->height = 1 + max_(height_(root->left), height_(root->right));

    return rebalance_(root);
}

static bool checkSentinels_(MemHeader *header) {
    return memcmp(
        header->sentinels,
        (uint8_t *)(header + 1) + header->blockSize,
        sizeof(uint64_t) * sentinelLen_
    ) == 0;
}

static void printHeaderInfo_(MemHeader *header) {
    fprintf(
        stderr,
        "%p - %s:%"PRIu32" - size=%zi\n",
        header + 1,
        header->file,
        header->line,
        header->blockSize
    );
}

static void printAllocations_(MemHeader *root) {
    if (root == NULL) {
        return;
    }
    printAllocations_(root->left);
    printHeaderInfo_(root);
    printAllocations_(root->right);
}

// Code found at https://stackoverflow.com/a/53900430/16275142

typedef struct PrngState {
    uint64_t state;
} PrngState;

static inline uint64_t prndSentinel_(PrngState *p) {
    uint64_t state = p->state;
    state ^= state >> 12;
    state ^= state << 25;
    state ^= state >> 27;
    p->state = state;
    return state * UINT64_C(2685821657736338717);
}

static void *memAllocFilled_(
    size_t byteCount,
    uint8_t val,
    uint32_t line,
    const char *file
) {
    MemHeader *block = malloc(
        sizeof(MemHeader)
        + byteCount
        + sizeof(uint64_t)*sentinelLen_
    );

    if (block == NULL) {
        fprintf(stderr, "out of memory");
        abort();
    }

    block->line = line;
    block->file = file;
    block->blockSize = byteCount;
    block->height = 1;
    block->left = NULL;
    block->right = NULL;

    PrngState state = { (uintptr_t)block };
    void *tailSentinels = (uint8_t *)(block + 1) + byteCount;
    for (int i = 0; i < sentinelLen_; i++) {
        uint64_t sentinel = prndSentinel_(&state);
        block->sentinels[i] = sentinel;
    }
    // cannot set tailSentinels[i] directly because the pointer might not be
    // aligned
    memcpy(tailSentinels, block->sentinels, sizeof(uint64_t)*sentinelLen_);

    memset((void *)(block + 1), val, byteCount);

    memRoot = insertHeader_(memRoot, block);
    return (void *)(block + 1);
}

void *memAlloc_(
    size_t objectCount,
    size_t objectSize,
    uint32_t line,
    const char *file
) {
    return memAllocFilled_(objectCount * objectSize, garbageByte_, line, file);
}

void *memAllocBytes_(size_t byteCount, uint32_t line, const char *file) {
    return memAllocFilled_(byteCount, garbageByte_, line, file);
}

void *memAllocZeroed_(
    size_t objectCount,
    size_t objectSize,
    uint32_t line,
    const char *file
) {
    return memAllocFilled_(objectCount * objectSize, 0, line, file);
}

void *memAllocZeroedBytes_(size_t byteCount, uint32_t line, const char *file) {
    return memAllocFilled_(byteCount, 0, line, file);
}

void *memExpand_(
    void *block,
    size_t newObjectCount,
    size_t objectSize,
    uint32_t line,
    const char *file
) {
    return memExpandBytes_(block, newObjectCount * objectSize, line, file);
}

void *memExpandBytes_(
    void *block,
    size_t newByteCount,
    uint32_t line,
    const char *file
) {
    assert(newByteCount != 0);
    MemHeader *header = (MemHeader *)block - 1;
    if (block != NULL && !hasHeader_(memRoot, header)) {
        fputs("memExpand: invalid pointer\n", stderr);
        fprintf(stderr, "   at %s:%x"PRIu32"\n", file, line);
        abort();
    }
    if (block != NULL && header->blockSize > newByteCount) {
        fprintf(stderr, "memExpand: new size (%zi) is smaller\n", newByteCount);
        fprintf(stderr, "   at %s:%"PRIu32"\n", file, line);
        printHeaderInfo_(header);
        abort();
    }
    return memChangeBytes_(block, newByteCount, line, file);
}

void *memShrink_(
    void *block,
    size_t newObjectCount,
    size_t objectSize,
    uint32_t line,
    const char *file
) {
    return memShrinkBytes_(block, newObjectCount * objectSize, line, file);
}

void *memShrinkBytes_(
    void *block,
    size_t newByteCount,
    uint32_t line,
    const char *file
) {
    assert(newByteCount != 0);
    MemHeader *header = (MemHeader *)block - 1;
    if (!hasHeader_(memRoot, header)) {
        fputs("memShrink: invalid pointer\n", stderr);
        fprintf(stderr, "   at %s:%"PRIu32"\n", file, line);
        abort();
    }
    if (header->blockSize < newByteCount) {
        fprintf(stderr, "memShrink: new size (%zi) is bigger\n", newByteCount);
        fprintf(stderr, "   at %s:%"PRIu32"\n", file, line);
        printHeaderInfo_(header);
        abort();
    }
    return memChangeBytes_(block, newByteCount, line, file);
}

void *memChange_(
    void *block,
    size_t objectCount,
    size_t objectSize,
    uint32_t line,
    const char *file
) {
    return memChangeBytes_(block, objectCount * objectSize, line, file);
}

void *memChangeBytes_(
    void *block,
    size_t byteCount,
    uint32_t line,
    const char *file
) {
    if (block == NULL) {
        return byteCount == 0 ? NULL : memAllocBytes_(byteCount, line, file);
    }

    MemHeader *header = (MemHeader *)block - 1;
    if (!hasHeader_(memRoot, header)) {
        fputs("memChange: invalid pointer\n", stderr);
        fprintf(stderr, "   at %s:%"PRIu32"\n", file, line);
        abort();
    }

    if (byteCount == 0) {
        memFree_(block, line, file);
        return NULL;
    }

    if (!checkSentinels_(header)) {
        fputs("memChange: out of bounds write\n", stderr);
        fprintf(stderr, "   at %s:%"PRIu32"\n", file, line);
        printHeaderInfo_(header);
        abort();
    }

    void *newBlock = memAllocFilled_(byteCount, garbageByte_, line, file);
    size_t minSize = byteCount < header->blockSize
        ? byteCount
        : header->blockSize;
    memcpy(newBlock, block, minSize);
    memFree(block);
    return newBlock;
}

void memFree_(void *block, uint32_t line, const char *file) {
    if (block == NULL) {
        return;
    }
    MemHeader *header = (MemHeader *)block - 1;
    if (!hasHeader_(memRoot, header)) {
        fputs("memFree: invalid pointer\n", stderr);
        fprintf(stderr, "   at %s:%"PRIu32"\n", file, line);
        abort();
    }

    if (!checkSentinels_(header)) {
        fputs("memFree: out of bounds write\n", stderr);
        fprintf(stderr, "   at %s:%"PRIu32"\n", file, line);
        printHeaderInfo_(header);
        abort();
    }
    memRoot = removeHeader_(memRoot, header);
    free(header);
}

bool memHasAllocs(void) {
    return memRoot != NULL;
}

void memPrintAllocs(void) {
    printAllocations_(memRoot);
}

#endif // !NDEBUG
