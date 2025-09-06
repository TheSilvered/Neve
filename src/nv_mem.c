#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "nv_mem.h"

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
