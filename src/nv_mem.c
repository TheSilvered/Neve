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

#define _sentinelLen 4
#define _garbageByte 0xc5

// Keep the headers in an AVL tree sorted by memory address

typedef struct MemHeader {
    struct MemHeader *left, *right;
    uint32_t height;
    uint32_t line;
    const char *file; // assume static storage for file names
    size_t blockSize;
    uint64_t sentinels[_sentinelLen]; // ps. rand. pattern for bounds checking
} MemHeader;

// Code for PRNG found at https://stackoverflow.com/a/53900430/16275142

typedef struct PrngState {
    uint64_t state;
} PrngState;

static inline uint64_t _prngNext(PrngState *p) {
    uint64_t state = p->state;
    state ^= state >> 12;
    state ^= state << 25;
    state ^= state >> 27;
    p->state = state;
    return state * UINT64_C(2685821657736338717);
}

thread_local MemHeader *t_memRoot = NULL;

static inline void _mhUpdateHeight(MemHeader *mh);
static inline int32_t _mhBalanceFactor(MemHeader *mh);
static MemHeader *_mhRotRight(MemHeader *oldRoot);
static MemHeader *_mhRotLeft(MemHeader *oldRoot);
static MemHeader *_mhRebalance(MemHeader *root);
static MemHeader *_mhInsert(MemHeader *root, MemHeader *mh);
static bool _mhContains(MemHeader *root, MemHeader *header);
static MemHeader *_mhMin(MemHeader *root);
static MemHeader *_mhRemove(MemHeader *root, MemHeader *mh);
static bool _mhCheckBounds(MemHeader *header);
static void _mhPrint(MemHeader *header);
static void _mhPrintAll(MemHeader *root);

static inline uint32_t _mhGetHeight(MemHeader *mh) {
    return mh ? mh->height : 0;
}

static inline void _mhUpdateHeight(MemHeader *mh) {
    uint32_t leftHeight = _mhGetHeight(mh->left);
    uint32_t rightHeight = _mhGetHeight(mh->right);
    mh->height = 1 + (leftHeight > rightHeight ? leftHeight : rightHeight);
}

static inline int32_t _mhBalanceFactor(MemHeader *mh) {
    return (int32_t)_mhGetHeight(mh->left) - (int32_t)_mhGetHeight(mh->right);
}

static MemHeader *_mhRotRight(MemHeader *oldRoot) {
    MemHeader *newRoot = oldRoot->left;
    MemHeader *tmp = newRoot->right;

    newRoot->right = oldRoot;
    oldRoot->left = tmp;

    _mhUpdateHeight(oldRoot);
    _mhUpdateHeight(newRoot);

    return newRoot;
}

static MemHeader *_mhRotLeft(MemHeader *oldRoot) {
    MemHeader *newRoot = oldRoot->right;
    MemHeader *tmp = newRoot->left;

    newRoot->left = oldRoot;
    oldRoot->right = tmp;

    _mhUpdateHeight(oldRoot);
    _mhUpdateHeight(newRoot);

    return newRoot;
}

static MemHeader *_mhRebalance(MemHeader *root) {
    int32_t balance = _mhBalanceFactor(root);

    if (balance > 1) {
        int32_t leftBalance = _mhBalanceFactor(root->left);
        if (leftBalance < 0) {
            root->left = _mhRotLeft(root->left);
        }
        return _mhRotRight(root);
    } else if (balance < -1) {
        int32_t rightBalance = _mhBalanceFactor(root->right);
        if (rightBalance > 0) {
            root->right = _mhRotRight(root->right);
        }
        return _mhRotLeft(root);
    } else {
        return root;
    }
}

static MemHeader *_mhInsert(MemHeader *root, MemHeader *mh) {
    if (root == NULL) {
        return mh;
    }

    assert(mh != root);
    if ((uintptr_t)mh < (uintptr_t)root) {
        root->left = _mhInsert(root->left, mh);
    } else {
        root->right = _mhInsert(root->right, mh);
    }
    _mhUpdateHeight(root);
    return _mhRebalance(root);
}

static bool _mhContains(MemHeader *root, MemHeader *header) {
    if (root == NULL) {
        return false;
    } else if ((uintptr_t)root == (uintptr_t)header) {
        return true;
    } else if ((uintptr_t)header < (uintptr_t)root) {
        return _mhContains(root->left, header);
    } else {
        return _mhContains(root->right, header);
    }
}

static MemHeader *_mhMin(MemHeader *root) {
    while (root->left) {
        root = root->left;
    }
    return root;
}

static MemHeader *_mhRemove(MemHeader *root, MemHeader *mh) {
    if ((uintptr_t)mh < (uintptr_t)root) {
        root->left = _mhRemove(root->left, mh);
    } else if ((uintptr_t)mh > (uintptr_t)root) {
        root->right = _mhRemove(root->right, mh);
    } else {
        if (!root->left) {
            return root->right;
        } else if (!root->right) {
            return root->left;
        }

        // Find smallest value of the right subtree and use it in place of root
        MemHeader *newRoot = _mhMin(root->right);
        newRoot->right = _mhRemove(root->right, newRoot);
        newRoot->left = root->left;
        root = newRoot;
    }
    _mhUpdateHeight(root);
    return _mhRebalance(root);
}

static bool _mhCheckBounds(MemHeader *header) {
    return 0 == memcmp(
        header->sentinels,
        (uint8_t *)(header + 1) + header->blockSize,
        sizeof(uint64_t) * _sentinelLen
    );
}

static void _mhPrint(MemHeader *header) {
    fprintf(
        stderr,
        "%p - %s:%"PRIu32" - size=%zi\n",
        header + 1,
        header->file,
        header->line,
        header->blockSize
    );
}

static void _mhPrintAll(MemHeader *root) {
    if (root == NULL) {
        return;
    }
    _mhPrintAll(root->left);
    _mhPrint(root);
    _mhPrintAll(root->right);
}

static void *_memAllocFilled(
    size_t byteCount,
    uint8_t val,
    uint32_t line,
    const char *file
) {
    MemHeader *block = malloc(
        sizeof(MemHeader)
        + byteCount
        + sizeof(uint64_t)*_sentinelLen
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
    for (int i = 0; i < _sentinelLen; i++) {
        uint64_t sentinel = _prngNext(&state);
        block->sentinels[i] = sentinel;
    }
    // cannot set tailSentinels[i] directly because the pointer might not be
    // aligned
    memcpy(tailSentinels, block->sentinels, sizeof(uint64_t)*_sentinelLen);

    memset((void *)(block + 1), val, byteCount);

    t_memRoot = _mhInsert(t_memRoot, block);
    return (void *)(block + 1);
}

void *_memAlloc(
    size_t objectCount,
    size_t objectSize,
    uint32_t line,
    const char *file
) {
    return _memAllocFilled(objectCount * objectSize, _garbageByte, line, file);
}

void *_memAllocBytes(size_t byteCount, uint32_t line, const char *file) {
    return _memAllocFilled(byteCount, _garbageByte, line, file);
}

void *_memAllocZeroed(
    size_t objectCount,
    size_t objectSize,
    uint32_t line,
    const char *file
) {
    return _memAllocFilled(objectCount * objectSize, 0, line, file);
}

void *_memAllocZeroedBytes(size_t byteCount, uint32_t line, const char *file) {
    return _memAllocFilled(byteCount, 0, line, file);
}

void *_memExpand(
    void *block,
    size_t newObjectCount,
    size_t objectSize,
    uint32_t line,
    const char *file
) {
    return _memExpandBytes(block, newObjectCount * objectSize, line, file);
}

void *_memExpandBytes(
    void *block,
    size_t newByteCount,
    uint32_t line,
    const char *file
) {
    assert(newByteCount != 0);
    MemHeader *header = (MemHeader *)block - 1;
    if (block != NULL && !_mhContains(t_memRoot, header)) {
        fputs("memExpand: invalid pointer\n", stderr);
        fprintf(stderr, "   at %s:%x"PRIu32"\n", file, line);
        abort();
    }
    if (block != NULL && header->blockSize > newByteCount) {
        fprintf(stderr, "memExpand: new size (%zi) is smaller\n", newByteCount);
        fprintf(stderr, "   at %s:%"PRIu32"\n", file, line);
        _mhPrint(header);
        abort();
    }
    return _memChangeBytes(block, newByteCount, line, file);
}

void *_memShrink(
    void *block,
    size_t newObjectCount,
    size_t objectSize,
    uint32_t line,
    const char *file
) {
    return _memShrinkBytes(block, newObjectCount * objectSize, line, file);
}

void *_memShrinkBytes(
    void *block,
    size_t newByteCount,
    uint32_t line,
    const char *file
) {
    assert(newByteCount != 0);
    MemHeader *header = (MemHeader *)block - 1;
    if (!_mhContains(t_memRoot, header)) {
        fputs("memShrink: invalid pointer\n", stderr);
        fprintf(stderr, "   at %s:%"PRIu32"\n", file, line);
        abort();
    }
    if (header->blockSize < newByteCount) {
        fprintf(stderr, "memShrink: new size (%zi) is bigger\n", newByteCount);
        fprintf(stderr, "   at %s:%"PRIu32"\n", file, line);
        _mhPrint(header);
        abort();
    }
    return _memChangeBytes(block, newByteCount, line, file);
}

void *_memChange(
    void *block,
    size_t objectCount,
    size_t objectSize,
    uint32_t line,
    const char *file
) {
    return _memChangeBytes(block, objectCount * objectSize, line, file);
}

void *_memChangeBytes(
    void *block,
    size_t byteCount,
    uint32_t line,
    const char *file
) {
    if (block == NULL) {
        return byteCount == 0 ? NULL : _memAllocBytes(byteCount, line, file);
    }

    MemHeader *header = (MemHeader *)block - 1;
    if (!_mhContains(t_memRoot, header)) {
        fputs("memChange: invalid pointer\n", stderr);
        fprintf(stderr, "   at %s:%"PRIu32"\n", file, line);
        abort();
    }

    if (byteCount == 0) {
        _memFree(block, line, file);
        return NULL;
    }

    if (!_mhCheckBounds(header)) {
        fputs("memChange: out of bounds write\n", stderr);
        fprintf(stderr, "   at %s:%"PRIu32"\n", file, line);
        _mhPrint(header);
        abort();
    }

    void *newBlock = _memAllocFilled(byteCount, _garbageByte, line, file);
    size_t minSize = byteCount < header->blockSize
        ? byteCount
        : header->blockSize;
    memcpy(newBlock, block, minSize);
    memFree(block);
    return newBlock;
}

void _memFree(void *block, uint32_t line, const char *file) {
    if (block == NULL) {
        return;
    }
    MemHeader *header = (MemHeader *)block - 1;
    if (!_mhContains(t_memRoot, header)) {
        fputs("memFree: invalid pointer\n", stderr);
        fprintf(stderr, "   at %s:%"PRIu32"\n", file, line);
        abort();
    }

    if (!_mhCheckBounds(header)) {
        fputs("memFree: out of bounds write\n", stderr);
        fprintf(stderr, "   at %s:%"PRIu32"\n", file, line);
        _mhPrint(header);
        abort();
    }
    t_memRoot = _mhRemove(t_memRoot, header);
    free(header);
}

bool memHasAllocs(void) {
    return t_memRoot != NULL;
}

void memPrintAllocs(void) {
    _mhPrintAll(t_memRoot);
}

void _memCheckBounds(void *block, uint32_t line, const char *file) {
    if (block == NULL) {
        return;
    }
    MemHeader *header = (MemHeader *)block - 1;
    if (!_mhContains(t_memRoot, header)) {
        fputs("memCheckBounds: invalid pointer\n", stderr);
        abort();
    }

    if (!_mhCheckBounds(header)) {
        fputs("memCheckBounds: out of bounds write\n", stderr);
        fprintf(stderr, "   at %s:%"PRIu32"\n", file, line);
        _mhPrint(header);
        abort();
    }
}

void memFreeAllAllocs(void) {
    while (t_memRoot != NULL) {
        memFree(t_memRoot + 1);
    }
}

#endif // !NDEBUG
