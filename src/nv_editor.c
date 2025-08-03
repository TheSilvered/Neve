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

void editorInit(void) {
    g_ed.rows = 0;
    g_ed.cols = 0;
    g_ed.rowBuffers = NULL;
    (void)strInit(&g_ed.screenBuf, 0);
    g_ed.tabStop = 8;
    g_ed.mode = EditorMode_Normal;
    g_ed.running = true;
    ctxInitNewFile(&g_ed.fileCtx, NULL);
    ctxInitLine(&g_ed.saveDialogCtx);

    strInitFromC(&g_ed.strings.savePrompt, "File path: ");

    termWrite(sLen(escCursorShapeStillBar));
}

void editorQuit(void) {
    strDestroy(&g_ed.screenBuf);

    if (g_ed.rowBuffers == NULL) {
        return;
    }

    for (uint16_t row = 0; row < g_ed.rows; row++) {
        strDestroy(&g_ed.rowBuffers[row].buf);
    }
    memFree(g_ed.rowBuffers);
}

bool editorSetRowCount_(uint16_t count) {
    if (count == g_ed.rows) {
        return true;
    } else if (count < g_ed.rows) {
        for (uint16_t row = count; row < g_ed.rows; row++) {
            strDestroy(&g_ed.rowBuffers[row].buf);
        }
        g_ed.rowBuffers = memShrink(
            g_ed.rowBuffers,
            count,
            sizeof(*g_ed.rowBuffers)
        );
        g_ed.rows = count;
        return true;
    } else {
        g_ed.rowBuffers = memChange(
            g_ed.rowBuffers,
            count,
            sizeof(*g_ed.rowBuffers)
        );
        for (uint16_t row = g_ed.rows; row < count; row++) {
            strInit(&g_ed.rowBuffers[row].buf, 0);
            g_ed.rowBuffers[row].changed = true;
        }
        g_ed.rows = count;
        return true;
    }
}

bool editorUpdateSize(void) {
    uint16_t rows, cols;
    if (!termSize(&rows, &cols)) {
        return false;
    }
    g_ed.cols = cols;
    editorSetRowCount_(rows);

    ctxSetWinSize(&g_ed.fileCtx, g_ed.cols, g_ed.rows - 1);

    if (g_ed.mode == EditorMode_SaveDialog) {
        // TODO: use visual width of savePrompt
        uint16_t saveDialogWidth = g_ed.cols - g_ed.strings.savePrompt.len;
        ctxSetWinSize(&g_ed.saveDialogCtx, saveDialogWidth, 1);
        g_ed.saveDialogCtx.win.termX = g_ed.strings.savePrompt.len;
        g_ed.saveDialogCtx.win.termY = g_ed.rows - 1;
    }

    return true;
}

bool editorDraw(uint16_t rowIdx, const UcdCh8 *buf, size_t len) {
    assert(rowIdx < g_ed.rows);
    StrView sv = { .buf = (const UcdCh8 *)buf, .len = len };
    strAppend(&g_ed.rowBuffers[rowIdx].buf, &sv);
    g_ed.rowBuffers[rowIdx].changed = true;
    return true;
}

bool editorDrawFmt(uint16_t rowIdx, const char *fmt, ...) {
    char buf[FmtBufSize] = { 0 };
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, FmtBufSize, fmt, args);
    return editorDraw(rowIdx, (UcdCh8 *)buf, len);
}

bool editorDrawEnd(void) {
    strAppendC(&g_ed.screenBuf, escCursorHide escCursorSetPos("", ""));

    char posBuf[64] = { 0 };
    for (uint16_t rowIdx = 0; rowIdx < g_ed.rows; rowIdx++) {
        if (!g_ed.rowBuffers[rowIdx].changed) {
            continue;
        }
        snprintf(
            posBuf,
            64,
            escCursorSetPos("%u", "%u") escLineClear,
            rowIdx + 1, 1
        );
        strAppendC(&g_ed.screenBuf, posBuf);
        strAppend(&g_ed.screenBuf, (StrView *)&g_ed.rowBuffers[rowIdx].buf);
        strClear(&g_ed.rowBuffers[rowIdx].buf, g_ed.rowBuffers[rowIdx].buf.len);
        g_ed.rowBuffers[rowIdx].changed = false;
    }

    Ctx *ctx = editorGetActiveCtx();

    uint16_t curX, curY;
    ctxGetCurTermPos(ctx, &curX, &curY);

    snprintf(posBuf, 32, escCursorSetPos("%u", "%u"), curY + 1, curX + 1);
    strAppendC(&g_ed.screenBuf, posBuf);

    if (!termWrite(g_ed.screenBuf.buf, g_ed.screenBuf.len)) {
        return false;
    }
    strClear(&g_ed.screenBuf, g_ed.screenBuf.len);
    return true;
}

bool editorRefresh(void) {
    if (!editorUpdateSize()) {
        return false;
    }

    renderFile();
    renderStatusBar();

    if (!editorDraw(g_ed.rows - 1, sLen(escCursorShow))) {
        return false;
    }
    if (!editorDrawEnd()) {
        return false;
    }
    return true;
}

Ctx *editorGetActiveCtx(void) {
    if (g_ed.mode == EditorMode_SaveDialog) {
        return &g_ed.saveDialogCtx;
    }
    return &g_ed.fileCtx;
}

bool editorSaveFile(void) {
    if (g_ed.fileCtx.path.len == 0) {
        return false;
    }
    File file;
    FileIOResult result = fileOpen(
        &file,
        strAsC(&g_ed.fileCtx.path),
        FileMode_Write
    );
    if (result != FileIOResult_Success) {
        return false;
    }
    if (!ctxWriteToFile(&g_ed.fileCtx, &file)) {
        fileClose(&file);
        return false;
    }
    fileClose(&file);
    return true;
}
