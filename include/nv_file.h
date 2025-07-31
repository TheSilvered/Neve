#ifndef NV_FILE_H_
#define NV_FILE_H_

#include "nv_string.h"

// A file opened in the editor.
// `lines` is an array of indices in `content` of the start of each line.
// Do not read `lines` directly. Use `fileLine`, `fileLinePtr` or
// `fileGetLineIdx`.
typedef struct File {
    Str path;
    UcdCh8 *content;
    size_t contentLen;
    size_t contentCap;
    size_t *lines;
    size_t linesLen;
    size_t linesCap;
    bool saved;
} File;

typedef enum FileIOResult {
    FileIOResult_Success,
    FileIOResult_FileNotFound,
    FileIOResult_PermissionDenied,
    FileIOResult_OperationNotAllowed,
    FileIOResult_FileTooBig,
    FileIOResult_BadPath,
    FileIOResult_OtherIOError,
} FileIOResult;

// Create a new file without any contents
void fileInitEmpty(File *file);
// Load a file from disk (in UTF-8)
FileIOResult fileInitOpen(File *file, const char *path);

// Destroy the contents of a file
void fileDestroy(File *file);

// Get the content of a file as a string view.
StrView fileContent(const File *file);

// Get the number of lines in a file
size_t fileLineCount(const File *file);
// Get the line where `fileIdx` is.
size_t fileLineFromFileIdx(const File *file, size_t fileIdx);
// Get the line of a file.
// `lineIdx == 0` is the first line.
// Return value has `.buf == NULL` if the line is out of bounds.
StrView fileLine(const File *file, size_t lineIdx);
// Get a pointer to the first character of a line.
// `lineIdx == 0` is the first line.
// Return NULL if the line is out of bounds.
UcdCh8 *fileLinePtr(const File *file, size_t lineIdx);
// Get the index to the first character of a line inside `file->content`.
// `lineIdx == 0` is the first line.
// Return -1 if the line is out of bounds.
ptrdiff_t fileLineChIdx(const File *file, size_t lineIdx);

// Insert data into the file at position `idx`.
void fileInsert(File *file, size_t idx, const UcdCh8 *data, size_t len);
// Remove data from `startIdx` included to `endIdx` excluded.
void fileRemove(File *file, size_t startIdx, size_t endIdx);

// TODO: Save the contents of a file to the disk
// void fileSave(File *file);

#endif // !NV_FILE_H_
