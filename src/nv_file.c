#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nv_file.h"
#include "nv_mem.h"
#include "nv_string.h"

#define FILE_ARR_RESIZE_IMPL_(arrName)                                         \
    if (requiredLen == file->arrName##Len) {                                   \
        return;                                                                \
    } else if (requiredLen == 0) {                                             \
        memFree(file->arrName);                                                \
        file->arrName = NULL;                                                  \
        file->arrName##Cap = 0;                                                \
        file->arrName##Len = 0;                                                \
    } else if (requiredLen < file->arrName##Cap / 4) {                         \
        file->arrName = memShrink(                                             \
            file->arrName,                                                     \
            file->arrName##Cap / 2,                                            \
            sizeof(*file->arrName)                                             \
        );                                                                     \
        file->arrName##Cap /= 2;                                               \
    } else if (requiredLen > file->arrName##Cap) {                             \
        size_t newCap = requiredLen + requiredLen / 2;                         \
        file->arrName = memChange(                                             \
            file->arrName,                                                     \
            newCap,                                                            \
            sizeof(*file->arrName)                                             \
        );                                                                     \
        file->arrName##Cap = newCap;                                           \
    }

void fileInitEmpty(File *file) {
    (void)strInit(&file->path, 0);
    file->content = NULL;
    file->contentLen = 0;
    file->contentCap = 0;
    file->lines = NULL;
    file->linesLen = 0;
    file->linesCap = 0;
    file->saved = false;
}

FileIOResult fileInitOpen(File *file, const char *path) {
    // Guarantee a valid file even on failure.
    fileInitEmpty(file);

#ifdef _WIN32
    const wchar_t *wpath = tempWStr(path);
    FILE *fp = _wfopen(wpath, L"rb");
#else
    FILE *fp = fopen(path, "rb");
#endif // !_WIN32
    if (fp == NULL) {
        switch (errno) {
        case ENOMEM:
            fprintf(stderr, "Out of memory.");
            abort();
        case EACCES:
            return FileIOResult_PermissionDenied;
        case EFBIG:
        case EOVERFLOW:
            return FileIOResult_FileTooBig;
        case EINVAL:
        case ENAMETOOLONG:
            return FileIOResult_BadPath;
        case ELOOP:
        case ENOENT:
            return FileIOResult_FileNotFound;
        case EPERM:
            return FileIOResult_OperationNotAllowed;
        default:
            return FileIOResult_OtherIOError;
        }
    }

    strInitFromC(&file->path, path);

#define bufSize_ 4096
    UcdCh8 buf[bufSize_];
    size_t bytesRead = 0;

    do {
        bytesRead = fread(buf, 1, bufSize_, fp);
        fileInsertData(file, file->contentLen, buf, bytesRead);
    } while (bytesRead == bufSize_);
    (void)fclose(fp);
#undef bufSize_
    file->saved = true;
    return FileIOResult_Success;
}

void fileDestroy(File *file) {
    memFree(file->content);
    file->content = NULL;
    file->contentLen = 0;
    file->contentCap = 0;

    memFree(file->content);
    file->lines = NULL;
    file->linesLen = 0;
    file->linesCap = 0;

    strDestroy(&file->path);
    file->saved = false;
}

size_t fileLineCount(File *file) {
    return file->linesLen + 1;
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

static void fileResizeContent_(File *file, size_t requiredLen) {
    FILE_ARR_RESIZE_IMPL_(content)
}

static void fileResizeLines_(File *file, size_t requiredLen) {
    FILE_ARR_RESIZE_IMPL_(lines)
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

void fileInsertData(File *file, size_t idx, UcdCh8 *data, size_t len) {
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
    fileResizeContent_(file, file->contentLen + trueLen);
    fileResizeLines_(file, file->linesLen + lineCount);

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
        return;
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
}
