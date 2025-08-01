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
    termWrite(sLen(escCursorShapeStillBar));
    fileInitEmpty(&ed->file);
    ed->scrollX = 0;
    ed->scrollY = 0;
    ed->curX = 0;
    ed->curY = 0;
    ed->baseCurX = 0;
    ed->fileCurIdx = 0;
    ed->tabStop = 8;
    ed->mode = EditorMode_Normal;
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
    if (ed->curY >= ed->viewboxH + ed->scrollY) {
        ed->scrollY = ed->curY - ed->viewboxH + 1;
    } else if (ed->curY < ed->scrollY) {
        ed->scrollY = ed->curY;
    }

    if (ed->curX >= ed->viewboxW + ed->scrollX) {
        ed->scrollX = ed->curX - ed->viewboxW + 1;
    } else if (ed->curX < ed->scrollX) {
        ed->scrollX = ed->curX;
    }
}

bool editorUpdateSize(Editor *ed) {
    uint16_t rows, cols;
    if (!termSize(&rows, &cols)) {
        return false;
    }
    ed->cols = cols;
    editorSetRowCount_(ed, rows);

    return true;
}

void editorSetViewboxSize(Editor *ed, uint16_t width, uint16_t height) {
    ed->viewboxW = width;
    ed->viewboxH = height;
    editorUpdateViewbox_(ed);
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

    uint16_t termCurX = (uint16_t)(ed->curX - ed->scrollX) + 1;
    uint16_t termCurY = (uint16_t)(ed->curY - ed->scrollY) + 1;

    snprintf(posBuf, 32, escCursorSetPos("%u", "%u"), termCurY, termCurX);
    strAppendC(&ed->screenBuf, posBuf);

    if (!termWrite(ed->screenBuf.buf, ed->screenBuf.len)) {
        return false;
    }
    strClear(&ed->screenBuf, ed->screenBuf.len);
    return true;
}

void editorMoveCursorX(Editor *ed, ptrdiff_t dx) {
    if (dx == 0) {
        return;
    }

    // Move the cursor by dx characters, it may be more than dx columns.

    size_t baseLineIdx = fileLineChIdx(&ed->file, ed->curY);
    size_t lineIdx = ed->fileCurIdx - baseLineIdx;
    StrView line = fileLine(&ed->file, ed->curY);

    if (dx > 0) {
        ptrdiff_t i;
        for (
            i = strViewNext(&line, lineIdx, NULL);
            i != -1;
            i = strViewNext(&line, i, NULL)
        ) {
            if (--dx == 0) {
                break;
            }
        }
        if (i < 0) {
            lineIdx = line.len;
        } else {
            lineIdx = i;
        }
    } else {
        ptrdiff_t i;
        for (
            i = strViewPrev(&line, lineIdx, NULL);
            i != -1;
            i = strViewPrev(&line, i, NULL)
        ) {
            if (++dx == 0) {
                break;
            }
        }
        if (i < 0) {
            lineIdx = 0;
        } else {
            lineIdx = i;
        }
    }

    ed->fileCurIdx = baseLineIdx + lineIdx;

    // Find the actual column of the cursor

    line.len = lineIdx; // Get the width until lineIdx
    size_t width = 0;
    uint8_t tabStop = ed->tabStop;
    UcdCP cp = -1;
    for (
        ptrdiff_t i = strViewNext(&line, -1, &cp);
        i != -1;
        i = strViewNext(&line, i, &cp)
    ) {
        width += ucdCPWidth(cp, tabStop, width);
    }

    ed->curX = width;
    ed->baseCurX = width;

    editorUpdateViewbox_(ed);
}

void editorMoveCursorY(Editor *ed, ptrdiff_t dy) {
    if (dy == 0) {
        return;
    }

    size_t lineCount = fileLineCount(&ed->file);
    ptrdiff_t endY = (ptrdiff_t)ed->curY + dy;

    if (endY < 0) {
        endY = 0;
    } else if (endY >= (ptrdiff_t)lineCount) {
        endY = lineCount - 1;
    }

    ed->curY = endY;

    StrView line = fileLine(&ed->file, ed->curY);
    size_t baseLineIdx = fileLineChIdx(&ed->file, endY);
    size_t lineIdx = line.len;

    size_t width = 0;
    uint8_t tabStop = ed->tabStop;
    UcdCP cp = -1;
    for (
        ptrdiff_t i = strViewNext(&line, -1, &cp);
        i != -1;
        i = strViewNext(&line, i, &cp)
    ) {
        uint8_t chWidth = ucdCPWidth(cp, tabStop, width);
        if (width + chWidth > ed->baseCurX) {
            lineIdx = i;
            break;
        }
        width += chWidth;
    }
    ed->curX = width;
    ed->fileCurIdx = baseLineIdx + lineIdx;

    editorUpdateViewbox_(ed);
}

void editorMoveCursorIdx(Editor *ed, ptrdiff_t diffIdx) {
    if (diffIdx == 0) {
        return;
    }

    StrView content = fileContent(&ed->file);
    size_t endCurIdx = 0;

    if (diffIdx > 0) {
        ptrdiff_t i;
        for (
            i = strViewNext(&content, ed->fileCurIdx, NULL);
            i != -1;
            i = strViewNext(&content, i, NULL)
        ) {
            if (--diffIdx == 0) {
                break;
            }
        }
        if (i < 0) {
            endCurIdx = content.len;
        } else {
            endCurIdx = i;
        }
    } else {
        ptrdiff_t i;
        for (
            i = strViewPrev(&content, ed->fileCurIdx, NULL);
            i != -1;
            i = strViewPrev(&content, i, NULL)
        ) {
            if (++diffIdx == 0) {
                break;
            }
        }
        if (i < 0) {
            endCurIdx = 0;
        } else {
            endCurIdx = i;
        }
    }

    size_t lineIdx = fileLineFromFileIdx(&ed->file, endCurIdx);
    size_t baseLineIdx = fileLineChIdx(&ed->file, lineIdx);
    StrView line = fileLine(&ed->file, lineIdx);

    line.len = endCurIdx - baseLineIdx;
    size_t width = 0;
    uint8_t tabStop = ed->tabStop;
    UcdCP cp = -1;
    for (
        ptrdiff_t i = strViewNext(&line, -1, &cp);
        i != -1;
        i = strViewNext(&line, i, &cp)
    ) {
        width += ucdCPWidth(cp, tabStop, width);
    }

    ed->curX = width;
    ed->baseCurX = width;
    ed->curY = lineIdx;
    ed->fileCurIdx = endCurIdx;

    editorUpdateViewbox_(ed);
}
