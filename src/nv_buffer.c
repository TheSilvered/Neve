#include <assert.h>
#include "nv_buffer.h"
#include "nv_file.h"

#define _mapMinCap 4

static BufHandle g_handleCounter = 1;

static void _bufMapInsert(BufMap *map, Buf buf);
static void _bufMapExpand(BufMap *map);
static void _bufMapShrink(BufMap *map);
static void _bufMapResize(BufMap *map, uint32_t newCap);

static void _bufMapInsert(BufMap *map, Buf buf) {
    // If the map is full for more than 2 thirds
    if (3*map->len >= 2*map->cap) {
        _bufMapExpand(map);
    }
    uint32_t mask = map->cap - 1;
    uint32_t idx = buf._handle & mask;
    for (uint32_t i = 0, cap = map->cap; i < cap; i++) {
        if (map->buffers[idx]._handle == bufInvalidHandle) {
            break;
        }
        idx = (idx + 1) & mask;
    }
    map->buffers[idx] = buf;
    map->len++;
}

static void _bufMapExpand(BufMap *map) {
    _bufMapResize(map, NV_MAX(map->cap*2, _mapMinCap));
}

static void _bufMapShrink(BufMap *map) {
    if (map->cap == 0 || map->cap < map->len * 4) {
        return;
    }
    _bufMapResize(map, NV_MAX(map->cap / 2, _mapMinCap));
}

static void _bufMapResize(BufMap *map, uint32_t newCap) {
    uint32_t oldCap = map->cap;
    Buf *oldBufs = map->buffers;
    map->buffers = memAllocZeroed(newCap, sizeof(*map->buffers));
    map->cap = newCap;

    // re-insert the values
    for (uint32_t i = 0; i < oldCap; i++) {
        if (oldBufs[i]._handle != bufInvalidHandle) {
            _bufMapInsert(map, oldBufs[i]);
        }
    }
    memFree(oldBufs);
}

void bufMapInit(BufMap *map) {
    *map = (BufMap) { 0 };
}

void bufMapDestroy(BufMap *map) {
    for (uint32_t i = 0; i < map->cap; i++) {
        if (map->buffers[i]._handle != bufInvalidHandle) {
            strDestroy(&map->buffers[i].path);
            ctxDestroy(&map->buffers[i].ctx);
        }
    }
    memFree(map->buffers);
    map->len = 0;
    map->cap = 0;
}

BufHandle bufInitEmpty(BufMap *map) {
    BufHandle handle = g_handleCounter++;
    Buf buf = { 0 };
    ctxInit(&buf.ctx, true);
    strInit(&buf.path, 0);
    buf._handle = handle;
    _bufMapInsert(map, buf);
    return handle;
}

FileIOResult bufInitFromFile(BufMap *map, File *file, BufHandle *outHandle) {
    assert(outHandle != NULL);

    Ctx ctx;
    ctxInit(&ctx, true);

    const size_t readBufSize = 4 * 1024 * 1024;
    UcdCh8 *readBuf = memAllocBytes(readBufSize);
    while (true) {
        size_t bytesRead;
        FileIOResult result = fileRead(file, readBuf, readBufSize, &bytesRead);
        if (result != FileIOResult_Success) {
            ctxDestroy(&ctx);
            *outHandle = bufInvalidHandle;
            return result;
        } else if (bytesRead != 0) {
            ctxAppend(&ctx, readBuf, bytesRead);
        } else {
            break;
        }
    }
    memFree(readBuf);

    BufHandle handle = g_handleCounter++;
    Buf buf = { 0 };
    buf.ctx = ctx;
    strInit(&buf.path, 0);
    buf._handle = handle;
    _bufMapInsert(map, buf);
    *outHandle = handle;
    return FileIOResult_Success;
}

Buf *bufRef(BufMap *map, BufHandle bufH) {
    uint32_t mask = (1 << map->cap) - 1;
    uint32_t idx = bufH & mask;
    for (uint32_t i = 0, cap = map->cap; i < cap; i++) {
        if (map->buffers[idx]._handle == bufInvalidHandle) {
            return NULL;
        } else if (map->buffers[idx]._handle == bufH) {
            return &map->buffers[idx];
        }
        idx = (idx + 1) & mask;
    }
    return NULL;
}

void bufClose(BufMap *map, BufHandle bufH) {
    uint32_t mask = map->cap - 1;
    uint32_t idx = bufH & mask;
    for (uint32_t i = 0, cap = map->cap; i < cap; i++) {
        if (map->buffers[idx]._handle == bufInvalidHandle) {
            return; // buffer not in the map
        } else if (map->buffers[idx]._handle == bufH) {
            break;
        }
        idx = (idx + 1) & mask;
    }
    if (map->buffers[idx]._handle != bufH) {
        return;
    }

    ctxDestroy(&map->buffers[idx].ctx);
    strDestroy(&map->buffers[idx].path);
    map->len--;

    idx = (idx + 1) & mask;
    for (uint32_t i = 0, cap = map->cap; i < cap; i++) {
        if (map->buffers[idx]._handle == bufInvalidHandle) {
            break;
        }
        Buf buf = map->buffers[idx];
        map->buffers[idx]._handle = bufInvalidHandle;
        _bufMapInsert(map, buf);
        idx = (idx + 1) & mask;
    }
    _bufMapShrink(map);
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
