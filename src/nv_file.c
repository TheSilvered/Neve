#include <errno.h>
#include <stdlib.h>
#include "nv_file.h"

#ifdef _WIN32
typedef wchar_t mode_char_t;
#define MODE_STR_(s) L##s
#else
typedef char mode_char_t;
#define MODE_STR_(s) s
#endif

FileIOResult fileOpen(File *file, const char *path, FileMode mode) {
    const mode_char_t *modeStr;
    switch (mode) {
    case FileMode_Read:
        modeStr = MODE_STR_("rb");
        break;
    case FileMode_Write:
        modeStr = MODE_STR_("wb");
        break;
    }

#ifdef _WIN32
    const wchar_t *wpath = tempWStr(path);
    FILE *fp = _wfopen(wpath, modeStr);
#else
    FILE *fp = fopen(path, modeStr);
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

    file->fp = fp;
    file->mode = mode;
    strInitFromC(&file->path, path);
    return FileIOResult_Success;
}

void fileClose(File *file) {
    if (file->fp == NULL) {
        return;
    }

    (void)fclose(file->fp);
    strDestroy(&file->path);
}

FileIOResult fileRead(
    File *file,
    uint8_t *outBuf,
    size_t bufSize,
    size_t *outBytesRead
) {
    if (file->fp == NULL || file->mode != FileMode_Read) {
        return FileIOResult_OperationNotAllowed;
    }

    size_t bytesRead = fread(outBuf, 1, bufSize, file->fp);
    if (outBytesRead != NULL) {
        *outBytesRead = bytesRead;
    }

    if (ferror(file->fp)) {
        return FileIOResult_OtherIOError;
    }
    return FileIOResult_Success;
}

FileIOResult fileWrite(File *file, const uint8_t *buf, size_t bufSize) {
    if (file->fp == NULL || file->mode != FileMode_Write) {
        return FileIOResult_OperationNotAllowed;
    }

    size_t bytesWritten = fwrite(buf, 1, bufSize, file->fp);
    if (bytesWritten < bufSize) {
        return FileIOResult_OtherIOError;
    }
    return FileIOResult_Success;
}
