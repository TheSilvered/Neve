#ifndef NV_FILE_H_
#define NV_FILE_H_

#include "nv_string.h"

// A file opened in the editor.
// `lines` is an array of indices in `content` of the start of each line.
// Do not read `lines` directly. Use `fileGetLine`, `fileGetLinePtr` or
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

// Create a new file without any contents
bool fileInitEmpty(File *file);
// Load a file from disk (in UTF-8)
bool fileInitOpen(File *file, const char *path);

// Destroy the contents of a file
void fileDestroy(File *file);

// Get the line of a file.
// `lineIdx == 0` is the first line.
// Return value has `.buf == NULL` if the line is out of bounds.
StrView fileGetLine(File *file, size_t lineIdx);

// Get a pointer to the first character of a line.
// `lineIdx == 0` is the first line.
// Return NULL if the line is out of bounds.
UcdCh8 *fileGetLinePtr(File *file, size_t lineIdx);

// Get the index to the first character of a line inside `file->content`.
// `lineIdx == 0` is the first line.
// Return -1 if the line is out of bounds.
ptrdiff_t fileGetLineChIdx(File *file, size_t lineIdx);

// Insert data into the file at position `idx`.
bool fileInsertData(File *file, size_t idx, UcdCh8 *data, size_t len);

// TODO: Save the contents of a file to the disk
// void fileSave(File *file);

#endif // !NV_FILE_H_
