#ifndef NV_FILE_H_
#define NV_FILE_H_

#include "nv_string.h"

// A file opened in the editor.
// `lines` is an array of indices in `content` of the start of each line.
typedef struct File {
    Str path;
    Str content; // TODO: manual content management (not Str)
    size_t *lines;
    size_t lineCount;
    bool saved;
} File;

// Create a new file without any contents
void fileInitEmpty(File *file);
// Load a file from disk (in UTF-8)
bool fileInitOpen(File *file, const char *path);

// TODO: Save the contents of a file to the disk
// void fileSave(File *file);

void fileDestroy(File *file);

#endif // !NV_FILE_H_
