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

#if __STDC_VERSION__ >= 201112L
    #if __STDC_VERSION__ < 202311L
        #define thread_local _Thread_local
    #endif
#elif defined(_MSC_VER) && !defined(__clang__)
#define thread_local __declspec( thread )
#else
#define thread_local __thread
#endif // !thread_local

#define sentinelLen_ 4
#define garbageByte_ 0xc5

// Keep the headers in an AVL tree sorted by memory address

typedef struct MemHeader {
    struct MemHeader *left, *right;
    uint32_t height;
    uint32_t line;
    const char *file; // assume static storage for file names
    size_t blockSize;
    uint64_t sentinels[sentinelLen_]; // ps. rand. pattern for bounds checking
} MemHeader;

// Code for PRNG found at https://stackoverflow.com/a/53900430/16275142

typedef struct PrngState {
    uint64_t state;
} PrngState;

static inline uint64_t prngNext_(PrngState *p) {
    uint64_t state = p->state;
    state ^= state >> 12;
    state ^= state << 25;
    state ^= state >> 27;
    p->state = state;
    return state * UINT64_C(2685821657736338717);
}

thread_local MemHeader *t_memRoot = NULL;

static inline void mhUpdateHeight_(MemHeader *mh);
static inline int32_t mhBalanceFactor_(MemHeader *mh);
static MemHeader *mhRotRight_(MemHeader *oldRoot);
static MemHeader *mhRotLeft_(MemHeader *oldRoot);
static MemHeader *mhRebalance_(MemHeader *root);
static MemHeader *mhInsert_(MemHeader *root, MemHeader *mh);
static bool mhContains_(MemHeader *root, MemHeader *header);
static MemHeader *mhMin_(MemHeader *root);
static MemHeader *mhRemove_(MemHeader *root, MemHeader *mh);
static bool mhCheckBounds_(MemHeader *header);
static void mhPrint_(MemHeader *header);
static void mhPrintAll_(MemHeader *root);

static inline uint32_t mhGetHeight_(MemHeader *mh) {
    return mh ? mh->height : 0;
}

static inline void mhUpdateHeight_(MemHeader *mh) {
    uint32_t leftHeight = mhGetHeight_(mh->left);
    uint32_t rightHeight = mhGetHeight_(mh->right);
    mh->height = 1 + (leftHeight > rightHeight ? leftHeight : rightHeight);
}

static inline int32_t mhBalanceFactor_(MemHeader *mh) {
    return (int32_t)mhGetHeight_(mh->left) - (int32_t)mhGetHeight_(mh->right);
}

static MemHeader *mhRotRight_(MemHeader *oldRoot) {
    MemHeader *newRoot = oldRoot->left;
    MemHeader *tmp = newRoot->right;

    newRoot->right = oldRoot;
    oldRoot->left = tmp;

    mhUpdateHeight_(oldRoot);
    mhUpdateHeight_(newRoot);

    return newRoot;
}

static MemHeader *mhRotLeft_(MemHeader *oldRoot) {
    MemHeader *newRoot = oldRoot->right;
    MemHeader *tmp = newRoot->left;

    newRoot->left = oldRoot;
    oldRoot->right = tmp;

    mhUpdateHeight_(oldRoot);
    mhUpdateHeight_(newRoot);

    return newRoot;
}

static MemHeader *mhRebalance_(MemHeader *root) {
    int32_t balance = mhBalanceFactor_(root);

    if (balance > 1) {
        int32_t leftBalance = mhBalanceFactor_(root->left);
        if (leftBalance < 0) {
            root->left = mhRotLeft_(root->left);
        }
        return mhRotRight_(root);
    } else if (balance < -1) {
        int32_t rightBalance = mhBalanceFactor_(root->right);
        if (rightBalance > 0) {
            root->right = mhRotRight_(root->right);
        }
        return mhRotLeft_(root);
    } else {
        return root;
    }
}

static MemHeader *mhInsert_(MemHeader *root, MemHeader *mh) {
    if (root == NULL) {
        return mh;
    }

    assert(mh != root);
    if ((uintptr_t)mh < (uintptr_t)root) {
        root->left = mhInsert_(root->left, mh);
    } else {
        root->right = mhInsert_(root->right, mh);
    }
    mhUpdateHeight_(root);
    return mhRebalance_(root);
}

static bool mhContains_(MemHeader *root, MemHeader *header) {
    if (root == NULL) {
        return false;
    } else if ((uintptr_t)root == (uintptr_t)header) {
        return true;
    } else if ((uintptr_t)header < (uintptr_t)root) {
        return mhContains_(root->left, header);
    } else {
        return mhContains_(root->right, header);
    }
}

static MemHeader *mhMin_(MemHeader *root) {
    while (root->left) {
        root = root->left;
    }
    return root;
}

static MemHeader *mhRemove_(MemHeader *root, MemHeader *mh) {
    if ((uintptr_t)mh < (uintptr_t)root) {
        root->left = mhRemove_(root->left, mh);
    } else if ((uintptr_t)mh > (uintptr_t)root) {
        root->right = mhRemove_(root->right, mh);
    } else {
        if (!root->left) {
            return root->right;
        } else if (!root->right) {
            return root->left;
        }

        // Find smallest value of the right subtree and use it in place of root
        MemHeader *newRoot = mhMin_(root->right);
        newRoot->right = mhRemove_(root->right, newRoot);
        newRoot->left = root->left;
        root = newRoot;
    }
    mhUpdateHeight_(root);
    return mhRebalance_(root);
}

static bool mhCheckBounds_(MemHeader *header) {
    return 0 == memcmp(
        header->sentinels,
        (uint8_t *)(header + 1) + header->blockSize,
        sizeof(uint64_t) * sentinelLen_
    );
}

static void mhPrint_(MemHeader *header) {
    fprintf(
        stderr,
        "%p - %s:%"PRIu32" - size=%zi\n",
        header + 1,
        header->file,
        header->line,
        header->blockSize
    );
}

static void mhPrintAll_(MemHeader *root) {
    if (root == NULL) {
        return;
    }
    mhPrintAll_(root->left);
    mhPrint_(root);
    mhPrintAll_(root->right);
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
        fprintf(stderr, "Out of memory.");
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
        uint64_t sentinel = prngNext_(&state);
        block->sentinels[i] = sentinel;
    }
    // cannot set tailSentinels[i] directly because the pointer might not be
    // aligned
    memcpy(tailSentinels, block->sentinels, sizeof(uint64_t)*sentinelLen_);

    memset((void *)(block + 1), val, byteCount);

    t_memRoot = mhInsert_(t_memRoot, block);
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
    if (block != NULL && !mhContains_(t_memRoot, header)) {
        fputs("memExpand: invalid pointer\n", stderr);
        fprintf(stderr, "   at %s:%x"PRIu32"\n", file, line);
        abort();
    }
    if (block != NULL && header->blockSize > newByteCount) {
        fprintf(stderr, "memExpand: new size (%zi) is smaller\n", newByteCount);
        fprintf(stderr, "   at %s:%"PRIu32"\n", file, line);
        mhPrint_(header);
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
    if (!mhContains_(t_memRoot, header)) {
        fputs("memShrink: invalid pointer\n", stderr);
        fprintf(stderr, "   at %s:%"PRIu32"\n", file, line);
        abort();
    }
    if (header->blockSize < newByteCount) {
        fprintf(stderr, "memShrink: new size (%zi) is bigger\n", newByteCount);
        fprintf(stderr, "   at %s:%"PRIu32"\n", file, line);
        mhPrint_(header);
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
    if (!mhContains_(t_memRoot, header)) {
        fputs("memChange: invalid pointer\n", stderr);
        fprintf(stderr, "   at %s:%"PRIu32"\n", file, line);
        abort();
    }

    if (byteCount == 0) {
        memFree_(block, line, file);
        return NULL;
    }

    if (!mhCheckBounds_(header)) {
        fputs("memChange: out of bounds write\n", stderr);
        fprintf(stderr, "   at %s:%"PRIu32"\n", file, line);
        mhPrint_(header);
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
    if (!mhContains_(t_memRoot, header)) {
        fputs("memFree: invalid pointer\n", stderr);
        fprintf(stderr, "   at %s:%"PRIu32"\n", file, line);
        abort();
    }

    if (!mhCheckBounds_(header)) {
        fputs("memFree: out of bounds write\n", stderr);
        fprintf(stderr, "   at %s:%"PRIu32"\n", file, line);
        mhPrint_(header);
        abort();
    }
    t_memRoot = mhRemove_(t_memRoot, header);
    free(header);
}

bool memHasAllocs(void) {
    return t_memRoot != NULL;
}

void memPrintAllocs(void) {
    mhPrintAll_(t_memRoot);
}

void memCheckBounds_(void *block, uint32_t line, const char *file) {
    if (block == NULL) {
        return;
    }
    MemHeader *header = (MemHeader *)block - 1;
    if (!mhContains_(t_memRoot, header)) {
        fputs("memCheckBounds: invalid pointer\n", stderr);
        abort();
    }

    if (!mhCheckBounds_(header)) {
        fputs("memCheckBounds: out of bounds write\n", stderr);
        fprintf(stderr, "   at %s:%"PRIu32"\n", file, line);
        mhPrint_(header);
        abort();
    }
}

void memFreeAllAllocs(void) {
    while (t_memRoot != NULL) {
        memFree(t_memRoot + 1);
    }
}

#endif // !NDEBUG
