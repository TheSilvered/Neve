#include <assert.h>
#include "nv_buffer.h"
#include "nv_file.h"

#define _mapMinCap 4

#ifndef _readBufSize
#define _readBufSize (4 * 1024 * 1024)
#endif // !_readBufSize

static BufHandle g_handleCounter = 1;

static void _bufFree(Buf *buf);

static void _bufMapInsert(BufMap *map, Buf *buf, BufHandle handle);
static void _bufMapExpand(BufMap *map);
static void _bufMapShrink(BufMap *map);
static void _bufMapResize(BufMap *map, uint32_t newCap);

static void _bufMapInsert(BufMap *map, Buf *buf, BufHandle handle) {
    // If the map is full for more than 2 thirds
    if (3*map->len >= 2*map->cap) {
        _bufMapExpand(map);
    }
    uint32_t mask = map->cap - 1;
    uint32_t idx = handle & mask;
    for (uint32_t i = 0, cap = map->cap; i < cap; i++) {
        if (map->_buckets[idx].handle == bufInvalidHandle) {
            break;
        }
        idx = (idx + 1) & mask;
    }
    map->_buckets[idx].buf = buf;
    map->_buckets[idx].handle = handle;
    map->len++;
}

static void _bufMapExpand(BufMap *map) {
    _bufMapResize(map, nvMax(map->cap*2, _mapMinCap));
}

static void _bufMapShrink(BufMap *map) {
    if (map->cap == 0 || map->cap < map->len * 4) {
        return;
    } else if (map->cap > _mapMinCap) {
        _bufMapResize(map, nvMax(map->cap / 2, _mapMinCap));
    }
}

static void _bufMapReinsert(BufMap *map, Buf *buf, BufHandle handle) {
    uint32_t mask = map->cap - 1;
    uint32_t idx = handle & mask;
    for (uint32_t i = 0, cap = map->cap; i < cap; i++) {
        if (map->_buckets[idx].handle == bufInvalidHandle) {
            break;
        }
        idx = (idx + 1) & mask;
    }
    map->_buckets[idx].buf = buf;
    map->_buckets[idx].handle = handle;
}

static void _bufMapResize(BufMap *map, uint32_t newCap) {
    uint32_t oldCap = map->cap;
    BufMapBucket *oldBuckets = map->_buckets;
    map->_buckets = memAllocZeroed(newCap, sizeof(*map->_buckets));
    map->cap = newCap;

    for (uint32_t i = 0; i < oldCap; i++) {
        if (oldBuckets[i].handle != bufInvalidHandle) {
            _bufMapReinsert(map, oldBuckets[i].buf, oldBuckets[i].handle);
        }
    }
    memFree(oldBuckets);
}

void bufMapInit(BufMap *map) {
    *map = (BufMap) { 0 };
}

void bufMapDestroy(BufMap *map) {
    for (uint32_t i = 0; i < map->cap; i++) {
        _bufFree(map->_buckets[i].buf);
    }
    memFree(map->_buckets);
    map->len = 0;
    map->cap = 0;
}

BufHandle bufInitEmpty(BufMap *map) {
    BufHandle handle = g_handleCounter++;
    Buf *buf = memAlloc(1, sizeof(Buf));
    ctxInit(&buf->ctx, true);
    ctxCurAdd(&buf->ctx, 0);
    strInit(&buf->path, 0);
    _bufMapInsert(map, buf, handle);
    return handle;
}

size_t _bufGetUTF8Part(Utf8Ch *buf, size_t len) {
    if (len == 0) {
        return 0;
    }
    // Find the last start byte and check if it is complete
    size_t idx = len - 1;
    while (idx > 0 && !utf8ChIsStart(buf[idx])) {
        idx--;
    }
    uint8_t runLen = utf8ChRunLen(buf[idx]);
    // If there was no valid start byte or there were extra invalid bytes at
    // the end
    if (runLen == 0 || idx + runLen < len) {
        return 0;
    } else if (idx + runLen == len) {
        return utf8Check(buf, len) ? len : 0;
    } else {
        return utf8Check(buf, idx) ? idx : 0;
    }
}

BufResult bufInitFromFile(BufMap *map, File *file, BufHandle *outHandle) {
    // TODO: skip BOM
    assert(outHandle != NULL);

    Ctx ctx;
    ctxInit(&ctx, true);

    const size_t readBufSize = _readBufSize;
    uint8_t *readBuf = memAllocBytes(readBufSize);
    size_t leftover = 0;

    BufResult ret = {
        .kind = BufResult_Success,
        .ioResult = FileIOResult_Success
    };

    while (true) {
        size_t bytesRead;
        FileIOResult result = fileRead(
            file,
            readBuf + leftover,
            readBufSize - leftover,
            &bytesRead
        );
        if (result != FileIOResult_Success) {
            ret.kind = BufResult_IOError;
            ret.ioResult = result;
            goto failure;
        } else if (bytesRead == 0 && leftover == 0) {
            break;
        }
        bytesRead += leftover;
        size_t validLen = _bufGetUTF8Part(readBuf, bytesRead);
        if (validLen == 0) {
            ret.kind = BufResult_EncodingError;
            goto failure;
        }
        leftover = bytesRead - validLen;
        ctxAppend(&ctx, readBuf, validLen);
        memmove(readBuf, readBuf + validLen, leftover);
    }
    memFree(readBuf);

    BufHandle handle = g_handleCounter++;
    Buf *buf = memAlloc(1, sizeof(Buf));
    buf->ctx = ctx;
    ctxCurAdd(&buf->ctx, 0);
    strInit(&buf->path, 0);
    _bufMapInsert(map, buf, handle);
    *outHandle = handle;
    return ret;

failure:
    memFree(readBuf);
    ctxDestroy(&ctx);
    *outHandle = bufInvalidHandle;
    return ret;
}

static void _bufFree(Buf *buf) {
    if (buf == NULL) {
        return;
    }
    ctxDestroy(&buf->ctx);
    strDestroy(&buf->path);
    memFree(buf);
}

Buf *bufRef(const BufMap *map, BufHandle bufH) {
    if (bufH == bufInvalidHandle) {
        return NULL;
    }
    uint32_t mask = map->cap - 1;
    uint32_t idx = bufH & mask;
    for (uint32_t i = 0, cap = map->cap; i < cap; i++) {
        if (map->_buckets[idx].handle == bufInvalidHandle) {
            return NULL;
        } else if (map->_buckets[idx].handle == bufH) {
            return map->_buckets[idx].buf;
        }
        idx = (idx + 1) & mask;
    }
    return NULL;
}

void bufClose(BufMap *map, BufHandle bufH) {
    if (bufH == bufInvalidHandle) {
        return;
    }

    uint32_t mask = map->cap - 1;
    uint32_t idx = bufH & mask;
    for (uint32_t i = 0, cap = map->cap; i < cap; i++) {
        if (map->_buckets[idx].handle == bufInvalidHandle) {
            return; // buffer not in the map
        } else if (map->_buckets[idx].handle == bufH) {
            break;
        }
        idx = (idx + 1) & mask;
    }
    if (map->_buckets[idx].handle != bufH) {
        return;
    }

    _bufFree(map->_buckets[idx].buf);
    map->_buckets[idx].handle = bufInvalidHandle;
    map->_buckets[idx].buf = NULL;
    map->len--;

    idx = (idx + 1) & mask;
    for (uint32_t i = 0, cap = map->cap; i < cap; i++) {
        if (map->_buckets[idx].handle == bufInvalidHandle) {
            break;
        }
        BufMapBucket bucket = map->_buckets[idx];
        map->_buckets[idx].handle = bufInvalidHandle;
        map->_buckets[idx].buf = NULL;
        _bufMapInsert(map, bucket.buf, bucket.handle);
        idx = (idx + 1) & mask;
    }
    _bufMapShrink(map);
}

bool bufSetPath(BufMap *map, BufHandle bufH, const char *path) {
    Buf *buf = bufRef(map, bufH);
    if (buf == NULL) {
        return false;
    }

    strDestroy(&buf->path);
    strInitFromC(&buf->path, path);
    return true;
}

FileIOResult bufWriteToDisk(Buf *buf) {
    if (buf->path.len == 0) {
        return FileIOResult_BadPath;
    }

    File file;
    FileIOResult result = fileOpen(&file, strAsC(&buf->path), FileMode_Write);
    if (result != FileIOResult_Success) {
        return result;
    }
    StrView content = ctxGetContent(&buf->ctx);
    result = fileWrite(&file, content.buf, content.len);
    fileClose(&file);
    return result;
}
