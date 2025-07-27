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
    ed->curX = 0;
    ed->curY = 0;
    ed->rows = 0;
    ed->cols = 0;
    ed->rowBuffers = NULL;
    (void)strInit(&ed->screenBuf, 0);
    termWrite(escWithLen(escCursorShapeStillBlock));
    fileInitEmpty(&ed->file);
    ed->fileLineOffset = 0;
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
    if (ed->curY >= ed->rows + ed->fileLineOffset) {
        ed->fileLineOffset = ed->curY - ed->rows + 1;
    } else if (ed->curY < ed->fileLineOffset) {
        ed->fileLineOffset = ed->curY;
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

    uint16_t termCurX = (uint16_t)ed->curX + 1;
    uint16_t termCurY = (uint16_t)(ed->curY - ed->fileLineOffset) + 1;

    snprintf(posBuf, 32, escCursorSetPos("%u", "%u"), termCurY, termCurX);
    strAppendC(&ed->screenBuf, posBuf);

    if (!termWrite(ed->screenBuf.buf, ed->screenBuf.len)) {
        return false;
    }
    strClear(&ed->screenBuf, ed->screenBuf.len);
    return true;
}

void editorMoveCursor(Editor *ed, ptrdiff_t dx, ptrdiff_t dy) {
    // TODO: add horizontal movement
    (void)dx;

    size_t lineCount = fileLineCount(&ed->file);
    ptrdiff_t endY = (ptrdiff_t)ed->curY + dy;
    if (endY >= (ptrdiff_t)lineCount) {
        endY = lineCount - 1;
    } else if ((ptrdiff_t)ed->curY + dy < 0) {
        endY = 0;
    }
    ed->curY = endY;
    ed->fileCurIdx = fileGetLineChIdx(&ed->file, endY);
    editorUpdateViewbox_(ed);
}

