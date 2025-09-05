#include <assert.h>
#include <stdarg.h>
#include <string.h>

#include "nv_context.h"
#include "nv_editor.h"
#include "nv_escapes.h"
#include "nv_mem.h"
#include "nv_screen.h"
#include "nv_term.h"
#include "nv_utils.h"

void screenInit(Screen *screen) {
    screen->w = 0;
    screen->h = 0;
    strInit(&screen->buf, 0);
    screen->editRows = NULL;
    screen->displayRows = NULL;
}

void screenDestroy(Screen *screen) {
    strDestroy(&screen->buf);
    for (uint16_t i = 0; i < screen->h; i++) {
        strDestroy(&screen->editRows[i]);
        strDestroy(&screen->displayRows[i]);
    }

    memFree(screen->editRows);
    memFree(screen->displayRows);
    screen->w = 0;
    screen->h = 0;
    screen->editRows = NULL;
    screen->displayRows = NULL;
}

void screenResize(Screen *screen, uint16_t w, uint16_t h) {
    if (h < screen->h) {
        for (uint16_t y = h; y < screen->h; y++) {
            strDestroy(&screen->editRows[y]);
            strDestroy(&screen->displayRows[y]);
        }
        screen->editRows = memShrink(
            screen->editRows,
            h,
            sizeof(*screen->editRows)
        );
        screen->displayRows = memShrink(
            screen->displayRows,
            h,
            sizeof(*screen->displayRows)
        );
    } else if (h > screen->h) {
        screen->editRows = memChange(
            screen->editRows,
            h,
            sizeof(*screen->editRows)
        );
        screen->displayRows = memChange(
            screen->displayRows,
            h,
            sizeof(*screen->displayRows)
        );
        for (uint16_t y = screen->h; y < h; y++) {
            strInit(&screen->editRows[y], 0);
            strInit(&screen->displayRows[y], 0);
        }
    }
    if (w != screen->w || h != screen->h) {
        screen->resized = true;
    } else {
        return;
    }

    screen->w = w;
    screen->h = h;
}

static size_t cutStr(StrView *sv, size_t maxWidth, size_t w) {
    ptrdiff_t i = 0;
    uint8_t cpWidth = 0;
    UcdCP cp = 0;
    for (
        i = strViewNext(sv, -1, &cp);
        i != -1;
        i = strViewNext(sv, i, &cp)
    ) {
        // Assume tabs have been filtered out
        cpWidth = ucdCPWidth(cp, 0, w);
        if (w + cpWidth > maxWidth) {
            break;
        }
        w += cpWidth;
    }

    if (i != -1) {
        sv->len = i;
    }
    return w;
}

void screenWrite(
    Screen *screen,
    uint16_t x, uint16_t y,
    const UcdCh8 *str, size_t len
) {
    if (x >= screen->w || y >= screen->h) {
        return;
    }

    uint16_t w = 0;
    uint16_t screenW = screen->w;
    Str *row = &screen->editRows[y];

    // Find the index in the string where the new string begins
    UcdCP cp = 0;
    uint8_t cpWidth = 0;
    ptrdiff_t startIdx = 0;
    for (
        startIdx = strViewNext((StrView *)row, -1, &cp);
        x != 0 && startIdx != -1;
        startIdx = strViewNext((StrView *)row, startIdx, &cp)
    ) {
        // Assume tabs have been filtered out
        cpWidth = ucdCPWidth(cp, 0, w);
        if (w + cpWidth > x) {
            break;
        }
        w += cpWidth;
    }

    uint16_t rowW = startIdx == -1 ? w : w + cpWidth; // Needed below

    // Append the part before the string to the new row
    StrView sv = {
        .buf = row->buf,
        .len = startIdx == -1 ? row->len : startIdx
    };
    strAppend(&screen->buf, &sv);
    // Pad with spaces if the boundary was in the middle of a character
    strRepeat(&screen->buf, ' ', x - w);

    // Add the string without going over the width of the screen
    sv.buf = str;
    sv.len = len;
    w = cutStr(&sv, screenW, w);
    strAppend(&screen->buf, &sv);

    if (w == screenW || startIdx == -1) {
        goto replaceRow;
    }

    // Add any characters that were after the string
    ptrdiff_t rowIdx = 0;
    for (
        rowIdx = strViewNext((StrView *)row, startIdx, &cp);
        rowIdx != -1 && rowW < w;
        rowIdx = strViewNext((StrView *)row, rowIdx, &cp)
    ) {
        rowW += ucdCPWidth(cp, 0, rowW);
    }

    if (rowIdx == -1) {
        goto replaceRow;
    }

    strRepeat(&screen->buf, ' ', rowW - w);

    sv.buf = row->buf + rowIdx;
    sv.len = row->len - rowIdx;
    (void)cutStr(&sv, screenW, rowW);
    strAppend(&screen->buf, &sv);

replaceRow:
    strClear(row, screen->buf.len);
    strAppend(row, (StrView *)&screen->buf);
    strClear(&screen->buf, screen->buf.cap);
}

void screenWriteFmt(
    Screen *screen,
    uint16_t x, uint16_t y,
    const char *fmt, ...
) {
    char buf[2048] = { 0 };
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, NV_ARRLEN(buf), fmt, args);
    va_end(args);
    if (len < 0) {
        return;
    }

    screenWrite(screen, x, y, (UcdCh8 *)buf, len);
}

void screenClear(Screen *screen, int32_t line) {
    if (line > screen->h) {
        return;
    }

    if (line >= 0) {
        strClear(&screen->editRows[line], screen->w);
        return;
    }
    for (uint16_t y = 0; y < screen->h; y++) {
        strClear(&screen->editRows[y], screen->w);
    }
}

static bool rowChanged(Str *editRow, Str *displayRow) {
    if (editRow->len != displayRow->len) {
        return true;
    }
    for (size_t i = 0, n = editRow->len; i < n; i++) {
        if (editRow->buf[i] != displayRow->buf[i]) {
            return true;
        }
    }
    return false;
}

bool screenRefresh(Screen *screen) {
    strAppendC(
        &screen->buf,
        escCursorHide
        escCursorSetPos("", "")
    );

    bool resized = screen->resized;

    if (resized) {
        strAppendC(&screen->buf, escScreenClear);
        screen->resized = false;
    }

    char posBuf[64] = { 0 };
    for (uint16_t y = 0; y < screen->h; y++) {
        Str *editRow = &screen->editRows[y];
        if (!resized && !rowChanged(editRow, &screen->displayRows[y])) {
            continue;
        }
        snprintf(
            posBuf,
            64,
            escCursorSetPos("%u", "%u") escLineClear,
            y + 1, 1
        );
        strAppendC(&screen->buf, posBuf);
        strAppend(&screen->buf, (StrView *)editRow);
    }

    Str *tempRows = screen->editRows;
    screen->editRows = screen->displayRows;
    screen->displayRows = tempRows;

    for (uint16_t y = 0; y < screen->h; y++) {
        strClear(&screen->editRows[y], screen->w);
    }

    Ctx *ctx = editorGetActiveCtx();

    uint16_t curX, curY;
    ctxGetCurTermPos(ctx, &curX, &curY);

    (void)snprintf(
        posBuf, 64,
        escCursorSetPos("%u", "%u"),
        curY + 1, curX + 1
    );

    strAppendC(&screen->buf, posBuf);
    strAppendC(&screen->buf, escCursorShow);

    if (!termWrite(screen->buf.buf, screen->buf.len)) {
        return false;
    }
    strClear(&screen->buf, screen->buf.len);

    return true;
}
