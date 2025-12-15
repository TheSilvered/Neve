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
    FileMode_Read     = 0x01, // rb
    FileMode_Write    = 0x02, // wb
    FileMode_Append   = 0x12, // ab
    FileMode_ReadEx   = 0x23, // r+b
    FileMode_WriteEx  = 0x33, // w+b
    FileMode_AppendEx = 0x43  // a+b
} FileMode;

typedef struct File {
    FileMode mode;
    Str path;
    FILE *fp;
} File;

// Open a file.
FileIOResult fileOpen(File *file, const char *path, FileMode mode);
// Make a temporary file.
FileIOResult fileOpenTemp(File *file);

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

// Move the file position indicator to the beginning.
bool filePosToBeginning(File *file);
// Move the file position indicator to the end.
bool filePosToEnd(File *file);
// Move the file position indicator by offset.
bool filePosMove(File *file, int64_t offset);
// Get the file position indicator.
int64_t filePosGet(File *file);

#endif // !NV_FILE_H_
