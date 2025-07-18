#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nv_file.h"
#include "nv_string.h"

#define FILE_ARR_RESIZE_IMPL_(arrName, type) \
    if (requiredLen == file->arrName##Len) { \
        return true; \
    } else if (requiredLen == 0) { \
        if (file->arrName != NULL) { \
            free(file->arrName); \
            file->arrName = NULL; \
        } \
        file->arrName##Cap = 0; \
        file->arrName##Len = 0; \
    } else if (requiredLen < file->arrName##Cap / 4) { \
        type *newData = realloc(file->arrName, sizeof(type) * (file->arrName##Cap / 2)); \
        if (newData == NULL) { \
            return true; \
        } \
        file->arrName = newData; \
        file->arrName##Cap /= 2; \
    } else if (requiredLen > file->arrName##Cap) { \
        type *newData = NULL; \
        size_t newCap = requiredLen + requiredLen / 2; \
        if (file->arrName == NULL) { \
            newData = malloc(sizeof(type) * newCap); \
        } else { \
            newData = realloc(file->arrName, sizeof(type) * newCap); \
        } \
        if (newData == NULL) { \
            return false; \
        } \
        file->arrName = newData; \
        file->arrName##Cap = newCap; \
    } \
    return true; \

bool fileInitEmpty(File *file) {
    (void)strInit(&file->path, 0);
    file->content = NULL;
    file->contentLen = 0;
    file->contentCap = 0;
    file->lines = NULL;
    file->linesLen = 0;
    file->linesCap = 0;
    file->saved = false;
    return true;
}

bool fileInitOpen(File *file, const char *path) {
    // Guarantee a valid file even on failure.
    fileInitEmpty(file);

#ifdef _WIN32
    const wchar_t *wpath = tempWStr(path);
    FILE *fp = _wfopen(wpath, L"rb");
#else
    FILE *fp = fopen(path, "rb");
#endif // !_WIN32
    if (fp == NULL) {
        return false;
    }

    if (!strInitFromC(&file->path, path)) {
        return false;
    }

#define bufSize_ 10
    UcdCh8 buf[bufSize_];
    size_t bytesRead = 0;

    do {
        bytesRead = fread(buf, 1, bufSize_, fp);
        if (!fileInsertData(file, file->contentLen, buf, bytesRead)) {
            fclose(fp);
            return false;
        }
    } while (bytesRead == bufSize_);
    fclose(fp);
#undef bufSize_
    file->saved = true;
    return true;
}

void fileDestroy(File *file) {
    strDestroy(&file->path);
    if (file->content != NULL) {
        free(file->content);
        file->content = NULL;
    }
    file->contentLen = 0;
    file->contentCap = 0;

    if (file->lines != NULL) {
        free(file->lines);
        file->lines = NULL;
    }
    file->linesLen = 0;
    file->linesCap = 0;
    file->saved = false;
}

StrView fileGetLine(File *file, size_t lineIdx) {
    UcdCh8 *buf = fileGetLinePtr(file, lineIdx);
    if (buf == NULL) {
        StrView empty = {
            .buf = NULL,
            .len = 0
        };
        return empty;
    }
    UcdCh8 *lineEnd = fileGetLinePtr(file, lineIdx + 1);
    if (lineEnd == NULL) {
        lineEnd = file->content + file->contentLen;
    }
    StrView sv = {
        .buf = buf,
        .len = lineEnd - buf
    };
    return sv;
}

UcdCh8 *fileGetLinePtr(File *file, size_t lineIdx) {
    ptrdiff_t idx = fileGetLineChIdx(file, lineIdx);
    if (idx == -1) {
        return NULL;
    }

    return file->content + idx;
}

ptrdiff_t fileGetLineChIdx(File *file, size_t lineIdx) {
    if (lineIdx > file->linesLen) {
        return -1;
    }
    if (lineIdx == 0) {
        return 0;
    } else {
        return file->lines[lineIdx - 1];
    }
}

static bool fileResizeContent_(File *file, size_t requiredLen) {
    FILE_ARR_RESIZE_IMPL_(content, UcdCh8)
}

static bool fileResizeLines_(File *file, size_t requiredLen) {
    FILE_ARR_RESIZE_IMPL_(lines, size_t)
}

static size_t fileFindLine_(File *file, size_t fileIdx) {
    if (file->linesLen == 0 || file->lines[0] > fileIdx) {
        return 0;
    } else if (fileIdx == file->contentLen) {
        return file->linesLen - 1;
    }

    size_t lo = 0;
    size_t hi = file->linesLen;

    while (lo + 1 != hi) {
        size_t idx = (hi + lo) / 2;
        size_t line = file->lines[idx];
        if (line < fileIdx) {
            lo = idx;
        } else if (line > fileIdx) {
            hi = idx;
        } else {
            return idx + 1;
        }
    }
    return lo + 1;
}

bool fileInsertData(File *file, size_t idx, UcdCh8 *data, size_t len) {
    size_t lineCount = 0;
    size_t trueLen = len;
    for (size_t i = 0; i < len; i++) {
        if (data[i] == '\n') {
            lineCount++;
        } else if (data[i] == '\r') {
            trueLen--;
        }
    }

    // Preallocate all necessary space
    if (!fileResizeContent_(file, file->contentLen + trueLen)) {
        return false;
    }

    if (!fileResizeLines_(file, file->linesLen + lineCount)) {
        return false;
    }

    UcdCh8 *content = file->content;

    // Insert the data in the contents
    memmove(
        content + idx + trueLen,
        content + idx,
        file->contentLen - idx
    );

    size_t copyIdx = idx;
    for (size_t i = 0; i < len; i++) {
        // Normalize all line endings to `\n`
        if (data[i] == '\r') {
            continue;
        }
        content[copyIdx++] = data[i];
    }
    file->contentLen += trueLen;

    // Update the line indices

    size_t *lines = file->lines;

    // Offset the indices after the data
    size_t lineIdx = fileFindLine_(file, idx);
    for (int i = lineIdx; i < file->linesLen; i++) {
        lines[i] += len;
    }

    if (lineCount == 0) {
        return true;
    }

    // Make space for the new lines
    memmove(
        lines + lineIdx + lineCount,
        lines + lineIdx,
        file->linesLen - lineIdx
    );
    file->linesLen += lineCount;

    // Add the new lines
    for (size_t i = 0; i < trueLen && lineCount != 0; i++) {
        if (content[i + idx] == '\n') {
            lines[lineIdx++] = i + idx + 1;
            lineCount--;
        }
    }
    return true;
}
