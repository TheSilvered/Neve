#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include "nv_editor.h"
#include "nv_escapes.h"
#include "nv_file.h"
#include "nv_mem.h"
#include "nv_term.h"

#define FmtBufSize 2048

Editor g_ed = { 0 };

void editorInit(Editor *ed) {
    ed->rows = 0;
    ed->cols = 0;
    ed->rowBuffers = NULL;
    (void)strInit(&ed->screenBuf, 0);
    termWrite(escWithLen(escCursorShapeStillBar));
    fileInitEmpty(&ed->file);
    ed->viewboxX = 0;
    ed->viewboxY = 0;
    ed->curX = 0;
    ed->curY = 0;
    ed->baseCurX = 0;
    ed->fileCurIdx = 0;
    ed->running = true;
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

void editorUpdateViewbox_(Editor *ed) {
    if (ed->curY >= ed->rows + ed->viewboxY) {
        ed->viewboxY = ed->curY - ed->rows + 1;
    } else if (ed->curY < ed->viewboxY) {
        ed->viewboxY = ed->curY;
    }

    if (ed->curX >= ed->cols + ed->viewboxX) {
        ed->viewboxX = ed->curX - ed->cols + 1;
    } else if (ed->curX < ed->viewboxX) {
        ed->viewboxX = ed->curX;
    }
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
    ed->cols = cols;
    editorSetRowCount_(ed, rows);

    editorUpdateViewbox_(ed);

    return true;
}

bool editorDraw(Editor *ed, uint16_t rowIdx, const char *buf, size_t len) {
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
    return editorDraw(ed, rowIdx, buf, len);
}

bool editorDrawEnd(Editor *ed) {
    char posBuf[32] = { 0 };
    for (uint16_t rowIdx = 0; rowIdx < ed->rows; rowIdx++) {
        if (!ed->rowBuffers[rowIdx].changed) {
            continue;
        }
        snprintf(posBuf, 32, escCursorSetPos("%u", "%u"), rowIdx + 1, 1);
        strAppendC(&ed->screenBuf, posBuf);
        strAppend(&ed->screenBuf, (StrView *)&ed->rowBuffers[rowIdx].buf);
        strClear(&ed->rowBuffers[rowIdx].buf, ed->rowBuffers[rowIdx].buf.len);
        ed->rowBuffers[rowIdx].changed = false;
    }

    uint16_t termCurX = (uint16_t)(ed->curX - ed->viewboxX) + 1;
    uint16_t termCurY = (uint16_t)(ed->curY - ed->viewboxY) + 1;

    snprintf(posBuf, 32, escCursorSetPos("%u", "%u"), termCurY, termCurX);
    strAppendC(&ed->screenBuf, posBuf);

    if (!termWrite(ed->screenBuf.buf, ed->screenBuf.len)) {
        return false;
    }
    strClear(&ed->screenBuf, ed->screenBuf.len);
    return true;
}

void editorMoveCursor(Editor *ed, ptrdiff_t dx, ptrdiff_t dy) {
    // Execute first the vertical movement and then the horizontal movement
    size_t lineCount = fileLineCount(&ed->file);
    ptrdiff_t endY = (ptrdiff_t)ed->curY + dy;

    if (endY < 0) {
        endY = 0;
    } else if (endY >= (ptrdiff_t)lineCount) {
        endY = lineCount - 1;
    }

    ed->curY = endY;

    ptrdiff_t endX = 0;
    if (dx != 0) {
        endX = (ptrdiff_t)ed->curX + dx;
    } else {
        endX = ed->baseCurX;
    }

    StrView line = fileGetLine(&ed->file, endY);

    if (endX < 0) {
        ed->fileCurIdx = fileGetLineChIdx(&ed->file, endY);
        endX = 0;
    } else {
        size_t baseIdx = fileGetLineChIdx(&ed->file, endY);
        size_t width = 0;
        UcdCP cp;
        ptrdiff_t lineIdx = -1;
        for (
            lineIdx = strViewNext(&line, lineIdx, &cp);
            lineIdx != -1;
            lineIdx = strViewNext(&line, lineIdx, &cp)
        ) {
            if (cp == '\n') {
                break;
            }
            uint8_t cpWidth = ucdCPWidth(cp);
            if (width + cpWidth > (size_t)endX) {
                break;
            }
            width += cpWidth;
        }
        endX = width;
        ed->fileCurIdx = baseIdx + lineIdx;
    }

    ed->curX = endX;
    if (dx != 0) {
        ed->baseCurX = ed->curX;
    }

    editorUpdateViewbox_(ed);
}

