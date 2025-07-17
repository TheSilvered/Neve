#ifndef NV_FILE_H_
#define NV_FILE_H_

#include <stdio.h>
#include "nv_string.h"

typedef struct File {
    Str path;
    Str content;
    size_t *lines;
    size_t lineCount;
    bool saved;
} File;

// Create a new file without any contents
void fileInitEmpty(File *file);
// TODO: Load a file from disk (in UTF-8)
// bool fileInitOpen(File *file, const char *path);

// TODO: Save the contents of a file to the disk
// void fileSave(File *file);

void fileDestroy(File *file);

#endif // !NV_FILE_H_
