#ifndef NV_BUFFER_H_
#define NV_BUFFER_H_

#include "nv_file.h"
#include "nv_context.h"

#define bufInvalidHandle ((BufHandle)0)

typedef uint32_t BufHandle;

// Open file buffer.
typedef struct Buf {
    Ctx ctx;
    Str path;
    BufHandle _handle;
} Buf;

typedef struct BufMap {
    // Not guaranteed to be continuous
    Buf *buffers;
    uint32_t len;
    uint32_t cap;
} BufMap;

// Initialize a buf map
void bufMapInit(BufMap *map);
// Destroy a buf map
void bufMapDestroy(BufMap *map);

// Initialize an empty buffer.
BufHandle bufNewEmpty(BufMap *map);
// Create a new buffer from an existing file.
FileIOResult bufInitFromFile(BufMap *map, File *file, BufHandle *outHandle);
// Get a reference to a buffer
Buf *bufRef(BufMap *map, BufHandle bufH);
// Close a buffer
void bufClose(BufMap *map, BufHandle bufH);

// Save the buffer if the path is valid.
FileIOResult bufWriteToDisk(Buf *buf);

#endif // !NV_BUFFER_H_
