#include <stdio.h>
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

bool fileInitOpen(File *file, const char *path) {
    // Guarantee a valid file even on failure.
    fileInitEmpty(file);

#ifdef _WIN32
    wchar_t wpath = tempWStr(path);
    FILE *f = _wfopen(wpath, L"rb");
#else
    FILE *f = fopen(path, "rb");
#endif // !_WIN32

    if (!strInitFromC(&file->path, path)) {
        return false;
    }

#define bufSize_ 4096
    UcdCh8 buf[bufSize_];
    size_t bytesRead = 0;

    do {
        bytesRead = fread(buf, 1, bufSize_, f);
        StrView sv = {
            .buf = buf,
            .len = bytesRead
        };
        if (!strAppend(&file->content, &sv)) {
            fileDestroy(file);
            return false;
        }
    } while (bytesRead == bufSize_);
#undef bufSize_

    size_t lineCount = 0;
    for (size_t i = 0; i < file->content.len; i++) {
        if (file->content.buf[i] == '\n') {
            lineCount++;
        }
    }
    size_t *lines = malloc(sizeof(size_t) * lineCount);
    if (lines == NULL) {
        fileDestroy(file);
        return false;
    }
    file->lines = lines;
    file->lineCount = lineCount;
    file->saved = true;
    return true;
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
