#include "nv_buffer.h"
#include "nv_file.h"

#define mapMinCap_ 4

static BufHandle g_handleCounter = 1;

static void bufMapInsert_(BufMap *map, Buf buf);
static void bufMapExpand_(BufMap *map);
static void bufMapShrink_(BufMap *map);
static void bufMapResize_(BufMap *map, uint32_t newCap);

static void bufMapInsert_(BufMap *map, Buf buf) {
    if (map->len == map->cap) {
        bufMapExpand_(map);
    }

    // if speed ever becomes an issue we will use a better algorithm but linear
    // search will do for now

    uint32_t mask = (1 << map->cap) - 1;
    uint32_t idx = buf._handle & mask;
    while (map->buffers[idx]._handle != bufInvalidHandle) {
        idx = (idx + 1) & mask;
    }
    map->buffers[idx] = buf;
}

static void bufMapExpand_(BufMap *map) {
    bufMapResize_(map, NV_MAX(map->cap*2, mapMinCap_));
}

static void bufMapShrink_(BufMap *map) {
    if (map->cap == 0 || map->cap < map->len * 4) {
        return;
    }
    bufMapResize_(map, NV_MAX(map->cap / 2, mapMinCap_));
}

static void bufMapResize_(BufMap *map, uint32_t newCap) {
    uint32_t oldCap = map->cap;
    Buf *oldBufs = map->buffers;
    map->buffers = memAllocZeroed(newCap, sizeof(*map->buffers));
    map->cap = newCap;

    // re-insert the values
    for (uint32_t i = 0; i < oldCap; i++) {
        if (oldBufs[i]._handle != bufInvalidHandle) {
            bufMapInsert_(map, oldBufs[i]);
        }
    }
    memFree(oldBufs);
}

void bufMapInit(BufMap *map) {
    *map = (BufMap) { 0 };
}

void bufMapDestroy(BufMap *map) {
    memFree(map->buffers);
    map->len = 0;
    map->cap = 0;
}

BufHandle bufNewEmpty(BufMap *map) {
    BufHandle handle = g_handleCounter++;
    Buf buf = { 0 };
    ctxInit(&buf.ctx, true);
    strInit(&buf.path, 0);
    buf._handle = handle;
    bufMapInsert_(map, buf);
    return handle;
}

BufHandle bufInitFromFile(BufMap *map, File *file) {
    Ctx ctx;
    ctxInit(&ctx, true);

    const size_t readBufSize = 4 * 1024 * 1024;
    UcdCh8 *readBuf = memAllocBytes(readBufSize);
    while (true) {
        size_t bytesRead;
        FileIOResult result = fileRead(file, readBuf, readBufSize, &bytesRead);
        if (result != FileIOResult_Success) {
            ctxDestroy(&ctx);
            return bufInvalidHandle;
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
    bufMapInsert_(map, buf);
    return handle;
}

// Get a reference to a buffer
Buf *bufRef(BufMap *map, BufHandle bufH);

// Close a buffer
void bufClose(BufMap *map, BufHandle bufH);

// Save the buffer if the path is valid.
bool bufWriteToDisk(BufMap *map, BufHandle bufH);
// Set the file path of the buffer.
void bufSetPath(BufMap *map, BufHandle bufH, StrView *path);
