#include <stdio.h>
#include <stdarg.h>

#include "nv_editor.h"
#include "nv_term.h"

#define FmtBufSize 2048

Editor g_ed;

void editorInit(Editor *ed) {
    ed->curX = 0;
    ed->curY = 0;
    (void)strInit(&ed->screenBuf, 0); // success guaranteed with reserve=0
}

void editorQuit(Editor *ed) {
    strDestroy(&ed->screenBuf);
}

bool editorDrawBegin(Editor *ed) {
    return termSize(&ed->rows, &ed->cols);
}

bool editorDraw(Editor *ed, const char *buf, size_t len) {
    StrView sv = {
        .buf = (const UcdCh8 *)buf,
        .len = len
    };
    return strAppend(&ed->screenBuf, &sv);
}

bool editorDrawFmt(Editor *ed, const char *fmt, ...) {
    const char *buf[FmtBufSize] = { 0 };
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, FmtBufSize, fmt, args);
    return editorDraw(ed, buf, len);
}

bool editorDrawEnd(Editor *ed) {
    if (!termWrite(ed->screenBuf.buf, ed->screenBuf.len)) {
        return false;
    }
    (void)strClear(&ed->screenBuf, ed->screenBuf.len); // success guaranteed
    ed->prevCols = ed->cols;
    ed->prevRows = ed->rows;
    return true;
}

bool editorColsChanged(Editor *ed) {
    return ed->cols != ed->prevCols;
}

bool editorRowsChanged(Editor *ed) {
    return ed->rows != ed->prevRows;
}
