#include <stdio.h>
#include <stdarg.h>

#include "nv_editor.h"
#include "nv_term.h"
#include "nv_escapes.h"

#define FmtBufSize 2048

Editor g_ed;

void editorInit(Editor *ed) {
    ed->curX = 0;
    ed->curY = 0;
    (void)strInit(&ed->screenBuf, 0); // success guaranteed with reserve=0
    termWrite(escWithLen(escCursorShapeStillBlock));
}

void editorQuit(Editor *ed) {
    strDestroy(&ed->screenBuf);
}

bool editorUpdateSize(Editor *ed, bool *outRowsChanged, bool *outColsChanged) {
    size_t rows, cols;
    if (!termSize(&rows, &cols)) {
        if (outRowsChanged != NULL) {
            *outRowsChanged = false;
        }
        if (outColsChanged != NULL) {
            *outColsChanged = false;
        }
        return false;
    }
    if (outRowsChanged != NULL) {
        *outRowsChanged = rows != ed->rows;
    }
    if (outColsChanged != NULL) {
        *outColsChanged = cols != ed->cols;
    }
    ed->rows = rows;
    ed->cols = cols;
    return true;
}

bool editorDraw(Editor *ed, const char *buf, size_t len) {
    StrView sv = {
        .buf = (const UcdCh8 *)buf,
        .len = len
    };
    return strAppend(&ed->screenBuf, &sv);
}

bool editorDrawFmt(Editor *ed, const char *fmt, ...) {
    char buf[FmtBufSize] = { 0 };
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, FmtBufSize, fmt, args);
    return editorDraw(ed, buf, len);
}

bool editorDrawEnd(Editor *ed) {
    if (ed->curX >= ed->cols) {
        ed->curX = ed->cols - 1;
    }
    if (ed->curY >= ed->rows) {
        ed->curY = ed->rows - 1;
    }

    editorDrawFmt(
        ed,
        escCursorSetPos("%zi", "%zi"),
        ed->curY + 1,
        ed->curX + 1
    );

    if (!termWrite(ed->screenBuf.buf, ed->screenBuf.len)) {
        return false;
    }
    (void)strClear(&ed->screenBuf, ed->screenBuf.len); // success guaranteed
    return true;
}
