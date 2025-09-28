#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#include "nv_context.h"
#include "nv_editor.h"
#include "nv_escapes.h"
#include "nv_mem.h"
#include "nv_screen.h"
#include "nv_string.h"
#include "nv_term.h"
#include "nv_utils.h"

void screenInit(Screen *screen) {
    screen->w = 0;
    screen->h = 0;
    strInit(&screen->buf, 0);
    screen->editRows = NULL;
    screen->displayRows = NULL;
    screen->editStyles = NULL;
    screen->displayStyles = NULL;
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

ScreenStyle *resizeStyles_(ScreenStyle *styles, size_t oldLen, size_t newLen) {
    if (oldLen == newLen) {
        return styles;
    }
    if (oldLen > newLen) {
        return memShrink(styles, newLen, sizeof(*styles));
    } else {
        ScreenStyle *newStyles = memChange(styles, newLen, sizeof(*styles));
        memset(newStyles + oldLen, 0, (newLen - oldLen) * sizeof(*styles));
        return newStyles;
    }
}

Str *resizeRows_(Str *rows, size_t oldLen, size_t newLen) {
    if (oldLen == newLen) {
        return rows;
    }
    if (oldLen > newLen) {
        for (uint16_t i = newLen; i < oldLen; i++) {
            strDestroy(&rows[i]);
        }
        return memShrink(rows, newLen, sizeof(*rows));
    } else {
        Str *newRows = memChange(rows, newLen, sizeof(*rows));
        for (uint16_t i = oldLen; i < newLen; i++) {
            strInit(&newRows[i], 0);
        }
        return newRows;
    }
}

void screenResize(Screen *screen, uint16_t w, uint16_t h) {
    // The terminal should not be resized too often, no need to have a buffered
    // allocation, just reallocate each time
    screen->editRows = resizeRows_(screen->editRows, screen->h, h);
    screen->displayRows = resizeRows_(screen->displayRows, screen->h, h);
    screen->editStyles = resizeStyles_(
        screen->editStyles,
        screen->w * screen->h,
        w * h
    );
    screen->displayStyles = resizeStyles_(
        screen->displayStyles,
        screen->w * screen->h,
        w * h
    );

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
        .len = startIdx < 0 ? row->len : (size_t)startIdx
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

NV_UNIX_FMT(4, 5) void screenWriteFmt(
    Screen *screen,
    uint16_t x, uint16_t y,
    NV_WIN_FMT const char *fmt, ...
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
        memset(
            &screen->editStyles[screen->w * line],
            0,
            screen->w * sizeof(*screen->editStyles)
        );
        return;
    }
    for (uint16_t y = 0; y < screen->h; y++) {
        strClear(&screen->editRows[y], screen->w);
    }
    memset(
        screen->editStyles,
        0,
        screen->w * screen->h * sizeof(*screen->editStyles)
    );
}

static bool rowChanged_(Screen *screen, uint16_t idx) {
    Str *editRow = &screen->editRows[idx];
    Str *displayRow = &screen->displayRows[idx];

    if (editRow->len != displayRow->len) {
        return true;
    }

    if (memcmp(editRow->buf, displayRow->buf, editRow->len) != 0) {
        return true;
    }
    return memcmp(
        &screen->editStyles[idx * screen->w],
        &screen->displayStyles[idx * screen->w],
        sizeof(*screen->editStyles) * screen->w
    );
}

void screenSetStyle(Screen *screen, ScreenStyle st, ScreenRect rect) {
    if (rect.x >= screen->w || rect.y >= screen->h) {
        return;
    }

    uint16_t maxX = rect.x + rect.w > screen->w ? screen->w : rect.x + rect.w;
    uint16_t maxY = rect.y + rect.h > screen->h ? screen->h : rect.y + rect.h;
    for (uint16_t y = rect.y; y < maxY; y++) {
        for (uint16_t x = rect.x; x < maxX; x++) {
            screen->editStyles[screen->w * y + x] = st;
        }
    }
}

static bool screenStyleEq_(ScreenStyle st1, ScreenStyle st2) {
    return memcmp(&st1, &st2, sizeof(st1)) == 0;
}

static void screenChangeStyle_(Screen *screen, ScreenStyle st) {
    char buf[32];
    strAppendC(&screen->buf, "\x1b[0");
    if (st.bold) {
        strAppendC(&screen->buf, ";1");
    }
    if (st.italic) {
        strAppendC(&screen->buf, ";3");
    }
    if (st.underline) {
        strAppendC(&screen->buf, ";4");
    }
    if (st.reverse) {
        strAppendC(&screen->buf, ";7");
    }
    if (st.strike) {
        strAppendC(&screen->buf, ";9");
    }
    if (st.colorMode == screenColorModeTerm16) {
        if (st.fg.r != 0) {
            snprintf(buf, NV_ARRLEN(buf), ";%d", st.fg.r + 29);
            strAppendC(&screen->buf, buf);
        }
        if (st.bg.r != 0) {
            snprintf(buf, NV_ARRLEN(buf), ";%d", st.bg.r + 39);
            strAppendC(&screen->buf, buf);
        }
    } else if (st.colorMode == screenColorModeTerm256) {
        snprintf(buf, NV_ARRLEN(buf), ";38;5;%d", st.fg.r);
        strAppendC(&screen->buf, buf);
        snprintf(buf, NV_ARRLEN(buf), ";48;5;%d", st.bg.r);
        strAppendC(&screen->buf, buf);
    } else if (st.colorMode == screenColorModeRGB) {
        snprintf(
            buf,
            NV_ARRLEN(buf),
            ";38;2;%d;%d;%d",
            st.fg.r, st.fg.g, st.fg.b
        );
        strAppendC(&screen->buf, buf);
        snprintf(
            buf,
            NV_ARRLEN(buf),
            ";48;2;%d;%d;%d",
            st.bg.r, st.bg.g, st.bg.b
        );
        strAppendC(&screen->buf, buf);
    }
    strAppendC(&screen->buf, "m");
}

static void writeLine_(Screen *screen, uint16_t idx) {
    char posBuf[64] = { 0 };
    StrView *editRow = (StrView *)&screen->editRows[idx];
    snprintf(
        posBuf,
        NV_ARRLEN(posBuf),
        escCursorSetPos("%u", "%u") escLineClear,
        idx + 1, 1
    );
    strAppendC(&screen->buf, posBuf);

    ScreenStyle currSt = { 0 };
    StrView span = {
        .buf = editRow->buf,
        .len = 0
    };
    size_t width = 0;
    UcdCP cp = 0;
    for (
        ptrdiff_t i = strViewNext(editRow, -1, &cp);
        i != -1;
        i = strViewNext(editRow, i, &cp)
    ) {
        ScreenStyle cellSt = screen->editStyles[idx * screen->w + width];
        width += ucdCPWidth(cp, 0, 0);
        if (!screenStyleEq_(cellSt, currSt)) {
            span.len = i - (span.buf - editRow->buf);
            strAppend(&screen->buf, &span);
            span.buf = editRow->buf + i;
            span.len = 0;
            currSt = cellSt;
            screenChangeStyle_(screen, cellSt);
        }
    }

    span.len = editRow->len - (span.buf - editRow->buf);
    strAppend(&screen->buf, &span);
    if (width < screen->w) {
        strRepeat(&screen->buf, ' ', screen->w - width);
    }

    screenChangeStyle_(screen, (ScreenStyle) { 0 });
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
    for (uint16_t i = 0; i < screen->h; i++) {
        if (!resized && !rowChanged_(screen, i)) {
            continue;
        }
        writeLine_(screen, i);
    }

    Str *tempRows = screen->editRows;
    screen->editRows = screen->displayRows;
    screen->displayRows = tempRows;

    ScreenStyle *tempStyles = screen->editStyles;
    screen->editStyles = screen->displayStyles;
    screen->displayStyles = tempStyles;

    for (uint16_t y = 0; y < screen->h; y++) {
        strClear(&screen->editRows[y], screen->w);
    }
    memset(
        screen->editStyles,
        0,
        sizeof(*screen->editStyles) * screen->w * screen->h
    );

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
