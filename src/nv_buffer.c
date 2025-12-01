#include "nv_buffer.h"
#include "nv_utils.h"

void bufInit(Buf *buf) {
    ctxInit(&buf->ctx, true);
    strInit(&buf->path, 0);
    buf->ctx.edited = true;
}

void bufDestroy(Buf *buf) {
    ctxDestroy(&buf->ctx);
    strDestroy(&buf->path);
}

bool bufInitFromFile(Buf *buf, File *file) {
    strInit(&buf->path, file->path.len);
    strAppend(&buf->path, (StrView *)&file->path);
    ctxInit(&buf->ctx, true);

    UcdCh8 readBuf[4096];
    size_t bytesRead = 0;

    do {
        FileIOResult result = fileRead(
            file,
            readBuf,
            NV_ARRLEN(readBuf),
            &bytesRead
        );
        if (result != FileIOResult_Success) {
            bufDestroy(buf);
            return false;
        }
        if (bytesRead != 0) {
            ctxAppend(&buf->ctx, readBuf, bytesRead);
        }
    } while (bytesRead == NV_ARRLEN(readBuf));
    buf->ctx.edited = false;
    return true;
}

bool bufWriteToDisk(Buf *buf) {
    if (buf->path.len == 0) {
        return false;
    }
    File file;
    FileIOResult ioResult = fileOpen(
        &file,
        strAsC(&buf->path),
        FileMode_Write
    );

    if (ioResult != FileIOResult_Success) {
        return false;
    }

    StrView content = ctxGetContent(&buf->ctx);

    ioResult = fileWrite(
        &file,
        content.buf,
        content.len
    );
    fileClose(&file);

    if (ioResult != FileIOResult_Success) {
        return false;
    }
    buf->ctx.edited = false;

    return true;
}

void bufSetPath(Buf *buf, StrView *path) {
    strClear(&buf->path, path->len);
    strAppend(&buf->path, path);
}
