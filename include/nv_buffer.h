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
} Buf;

typedef struct BufMapBucket {
    Buf *buf;
    BufHandle handle;
} BufMapBucket;

typedef struct BufMap {
    BufMapBucket *_buckets;
    uint32_t len;
    uint32_t cap;
} BufMap;

typedef enum BufResultKind {
    BufResult_Success,
    BufResult_IOError,
    BufResult_EncodingError
} BufResultKind;

typedef struct BufResult {
    BufResultKind kind;
    FileIOResult ioResult;
} BufResult;

// Initialize a buf map.
void bufMapInit(BufMap *map);
// Destroy a buf map.
void bufMapDestroy(BufMap *map);

// Initialize an empty buffer.
BufHandle bufInitEmpty(BufMap *map);
// Create a new buffer from an existing file.
BufResult bufInitFromFile(BufMap *map, File *file, BufHandle *outHandle);
// Get a reference to a buffer.
Buf *bufRef(const BufMap *map, BufHandle bufH);
// Close a buffer.
void bufClose(BufMap *map, BufHandle bufH);

// Change the path of a buffer.
bool bufSetPath(BufMap *map, BufHandle bufH, StrView *path);
// Change the path of a buffer using a C string.
bool bufSetPathC(BufMap *map, BufHandle bufH, const char *path);

// Save the buffer if the path is valid.
FileIOResult bufWriteToDisk(Buf *buf);

#endif // !NV_BUFFER_H_
