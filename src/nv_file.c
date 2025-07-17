#include <stdlib.h>
#include "nv_file.h"
#include "nv_string.h"

void fileInitEmpty(File *file) {
    (void)strInit(&file->path, 0);
    (void)strInit(&file->content, 0);
    file->lines = NULL;
    file->lineCount = 0;
    file->saved = false;
}

void fileDestroy(File *file) {
    strDestroy(&file->path);
    strDestroy(&file->content);
    if (file->lines != NULL) {
        free(file->lines);
        file->lines = NULL;
    }
    file->lineCount = 0;
    file->saved = false;
}
