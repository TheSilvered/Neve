#ifndef NV_BUFFER_H_
#define NV_BUFFER_H_

#include "nv_context.h"

// Open file buffer.
typedef struct Buf {
    Ctx ctx;
    Str path;
} Buf;

// Initialize an empty buffer.
void bufInit(Buf *buf);
// Create a new buffer from an existing file.
bool bufInitFromFile(Buf *buf, File *file);
// Destroy a buffer.
void bufDestroy(Buf *buf);

// Save the buffer if the path is valid.
bool bufWriteToDisk(Buf *buf);
// Set the file path of the buffer.
void bufSetPath(Buf *buf, StrView *path);

#endif // !NV_BUFFER_H_
