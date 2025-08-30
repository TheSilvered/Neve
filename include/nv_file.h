#ifndef NV_FILE_H_
#define NV_FILE_H_

#include <stdio.h>
#include "nv_string.h"

typedef enum FileIOResult {
    FileIOResult_Success,
    FileIOResult_FileNotFound,
    FileIOResult_PermissionDenied,
    FileIOResult_OperationNotAllowed,
    FileIOResult_FileTooBig,
    FileIOResult_BadPath,
    FileIOResult_OtherIOError,
} FileIOResult;

typedef enum FileMode {
    FileMode_Read,
    FileMode_Write
} FileMode;

typedef struct File {
    FileMode mode;
    Str path;
    FILE *fp;
} File;

// Open a file.
FileIOResult fileOpen(File *file, const char *path, FileMode mode);

// Close a file and free any associated memory.
void fileClose(File *file);

// Read from a file.
FileIOResult fileRead(
    File *file,
    uint8_t *outBuf,
    size_t bufSize,
    size_t *outBytesRead
);

// Write to a file.
FileIOResult fileWrite(File *file, const uint8_t *buf, size_t bufSize);

#endif // !NV_FILE_H_
