#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

#include "nv_editor.h"
#include "nv_escapes.h"
#include "nv_file.h"
#include "nv_mem.h"
#include "nv_render.h"
#include "nv_term.h"

#define FmtBufSize 2048

Editor g_ed = { 0 };

void editorInit(Editor *ed) {
    ed->rows = 0;
    ed->cols = 0;
    ed->rowBuffers = NULL;
    (void)strInit(&ed->screenBuf, 0);
    ed->tabStop = 8;
    ed->mode = EditorMode_Normal;
    ed->running = true;
    ctxInitNewFile(&ed->fileCtx, NULL);
    ctxInitLine(&ed->saveDialogCtx);

    strInitFromC(&ed->strings.savePrompt, "File path: ");

    termWrite(sLen(escCursorShapeStillBar));
}

void editorQuit(Editor *ed) {
    strDestroy(&ed->screenBuf);

    if (ed->rowBuffers == NULL) {
        return;
    }

    for (uint16_t row = 0; row < ed->rows; row++) {
        strDestroy(&ed->rowBuffers[row].buf);
    }
    memFree(ed->rowBuffers);
}

bool editorSetRowCount_(Editor *ed, uint16_t count) {
    if (count == ed->rows) {
        return true;
    } else if (count < ed->rows) {
        for (uint16_t row = count; row < ed->rows; row++) {
            strDestroy(&ed->rowBuffers[row].buf);
        }
        ed->rowBuffers = memShrink(
            ed->rowBuffers,
            count,
            sizeof(*ed->rowBuffers)
        );
        ed->rows = count;
        return true;
    } else {
        ed->rowBuffers = memChange(
            ed->rowBuffers,
            count,
            sizeof(*ed->rowBuffers)
        );
        for (uint16_t row = ed->rows; row < count; row++) {
            strInit(&ed->rowBuffers[row].buf, 0);
            ed->rowBuffers[row].changed = true;
        }
        ed->rows = count;
        return true;
    }
}

bool editorUpdateSize(Editor *ed) {
    uint16_t rows, cols;
    if (!termSize(&rows, &cols)) {
        return false;
    }
    ed->cols = cols;
    editorSetRowCount_(ed, rows);

    ctxSetWinSize(&ed->fileCtx, ed->cols, ed->rows - 1);

    if (ed->mode == EditorMode_SaveDialog) {
        // TODO: use visual width of savePrompt
        uint16_t saveDialogWidth = ed->cols - ed->strings.savePrompt.len;
        ctxSetWinSize(&ed->saveDialogCtx, saveDialogWidth, 1);
        ed->saveDialogCtx.win.termX = ed->strings.savePrompt.len;
        ed->saveDialogCtx.win.termY = ed->rows - 1;
    }

    return true;
}

bool editorDraw(Editor *ed, uint16_t rowIdx, const UcdCh8 *buf, size_t len) {
    assert(rowIdx < ed->rows);
    StrView sv = { .buf = (const UcdCh8 *)buf, .len = len };
    strAppend(&ed->rowBuffers[rowIdx].buf, &sv);
    ed->rowBuffers[rowIdx].changed = true;
    return true;
}

bool editorDrawFmt(Editor *ed, uint16_t rowIdx, const char *fmt, ...) {
    char buf[FmtBufSize] = { 0 };
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, FmtBufSize, fmt, args);
    return editorDraw(ed, rowIdx, (UcdCh8 *)buf, len);
}

bool editorDrawEnd(Editor *ed) {
    strAppendC(&ed->screenBuf, escCursorHide escCursorSetPos("", ""));

    char posBuf[64] = { 0 };
    for (uint16_t rowIdx = 0; rowIdx < ed->rows; rowIdx++) {
        if (!ed->rowBuffers[rowIdx].changed) {
            continue;
        }
        snprintf(
            posBuf,
            64,
            escCursorSetPos("%u", "%u") escLineClear,
            rowIdx + 1, 1
        );
        strAppendC(&ed->screenBuf, posBuf);
        strAppend(&ed->screenBuf, (StrView *)&ed->rowBuffers[rowIdx].buf);
        strClear(&ed->rowBuffers[rowIdx].buf, ed->rowBuffers[rowIdx].buf.len);
        ed->rowBuffers[rowIdx].changed = false;
    }

    Ctx *ctx = editorGetActiveCtx(ed);

    uint16_t curX, curY;
    ctxGetCurTermPos(ctx, &curX, &curY);

    snprintf(posBuf, 32, escCursorSetPos("%u", "%u"), curY + 1, curX + 1);
    strAppendC(&ed->screenBuf, posBuf);

    if (!termWrite(ed->screenBuf.buf, ed->screenBuf.len)) {
        return false;
    }
    strClear(&ed->screenBuf, ed->screenBuf.len);
    return true;
}

bool editorRefresh(Editor *ed) {
    if (!editorUpdateSize(ed)) {
        return false;
    }

    renderFile(ed);
    renderStatusBar(ed);

    if (!editorDraw(ed, ed->rows - 1, sLen(escCursorShow))) {
        return false;
    }
    if (!editorDrawEnd(ed)) {
        return false;
    }
    return true;
}

Ctx *editorGetActiveCtx(Editor *ed) {
    if (ed->mode == EditorMode_SaveDialog) {
        return &ed->saveDialogCtx;
    }
    return &ed->fileCtx;
}

bool editorSaveFile(Editor *ed) {
    if (ed->fileCtx.path.len == 0) {
        return false;
    }
    File file;
    FileIOResult result = fileOpen(
        &file,
        strAsC(&ed->fileCtx.path),
        FileMode_Write
    );
    if (result != FileIOResult_Success) {
        return false;
    }
    if (!ctxWriteToFile(&ed->fileCtx, &file)) {
        fileClose(&file);
        return false;
    }
    fileClose(&file);
    return true;
}
