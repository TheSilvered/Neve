#include <assert.h>
#include <stdarg.h>
#include <string.h>

#include "nv_context.h"
#include "nv_editor.h"
#include "nv_escapes.h"
#include "nv_mem.h"
#include "nv_screen.h"
#include "nv_term.h"

#define ScreenFmtBufSize_ 2048

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
        cpWidth = ucdCPWidth(cp, g_ed.tabStop, w);
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
        cpWidth = ucdCPWidth(cp, g_ed.tabStop, w);
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
        rowW += ucdCPWidth(cp, g_ed.tabStop, rowW);
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
    char buf[ScreenFmtBufSize_] = { 0 };
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, ScreenFmtBufSize_, fmt, args);
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

#define FmtBufSize_ 32

static void renderLine_(
    const StrView *line,
    size_t maxWidth,
    size_t scrollX,
    Str *outBuf
) {
    if (maxWidth == 0) {
        return;
    }

    strClear(outBuf, maxWidth);
    char fmtBuf[FmtBufSize_];
    StrView fmtView = { .buf = (UcdCh8 *)fmtBuf, .len = 0 };

    size_t width = 0;

    const char *tabFmt =
        escSetStyle(colorBrightBlackFg)
        "\xc2\xbb%*s" // »%*s
        escSetStyle(styleDefault);
    const char *startCutoffFmt =
        escSetStyle(colorBrightBlackFg)
        "<%*s"
        escSetStyle(styleDefault);
    const char *endCutoffFmt =
        escSetStyle(colorBrightBlackFg)
        "%*s>"
        escSetStyle(styleDefault);

    UcdCP cp = -1;
    for (
        ptrdiff_t i = strViewNext(line, -1, &cp);
        i != -1;
        i = strViewNext(line, i, &cp)
    ) {
        uint8_t chWidth = ucdCPWidth(cp, g_ed.tabStop, width);
        width += chWidth;

        if (width <= scrollX) {
            continue;
        } else if (width - chWidth < scrollX) {
            // Draw a gray '<' at the start if a character is cut off
            fmtView.len = snprintf(
                fmtBuf, FmtBufSize_,
                startCutoffFmt,
                width - scrollX - 1, ""
            );
            strAppend(outBuf, &fmtView);
        } else if (width > maxWidth) {
            // Draw a gray '>' at the end if a character is cut off
            // If the character is a tab it can just draw a '»'
            fmtView.len = snprintf(
                fmtBuf, FmtBufSize_,
                cp == '\t' ? tabFmt : endCutoffFmt,
                maxWidth + chWidth - width - 1, ""
            );
            strAppend(outBuf, &fmtView);
            break;
        } else if (cp == '\t') {
            fmtView.len = snprintf(
                fmtBuf, FmtBufSize_,
                tabFmt,
                chWidth - 1, ""
            );
            strAppend(outBuf, &fmtView);
        } else {
            StrView sv = {
                .buf = line->buf + i,
                .len = ucdCh8CPLen(cp)
            };
            strAppend(outBuf, &sv);
        }

        if (width == maxWidth) {
            break;
        }
    }
}

void renderFile(void) {
    Str lineBuf = { 0 };
    for (uint16_t i = 0; i < g_ed.fileCtx.win.h; i++) {
        if (i + g_ed.fileCtx.win.y < ctxLineCount(&g_ed.fileCtx)) {
            StrView line = ctxGetLine(&g_ed.fileCtx, i + g_ed.fileCtx.win.y);
            renderLine_(
                &line,
                g_ed.fileCtx.win.w,
                g_ed.fileCtx.win.x,
                &lineBuf
            );
            screenClear(&g_ed.screen, i);
            screenWrite(&g_ed.screen, 0, i, lineBuf.buf, lineBuf.len);
            continue;
        }

        screenWrite(&g_ed.screen, 0, i, sLen("~"));
    }

    if (g_ed.fileCtx.text.bufLen != 0) {
        return;
    }

    StrView msg = { sLen("Neve editor prototype") };
    screenWrite(
        &g_ed.screen,
        (g_ed.screen.w - msg.len) / 2,
        g_ed.fileCtx.win.h / 2,
        msg.buf, msg.len
    );
}

void renderSaveDialog_(void) {
    screenWrite(
        &g_ed.screen,
        0, g_ed.screen.h - 1,
        g_ed.strings.savePrompt.buf,
        g_ed.strings.savePrompt.len
    );

    StrView path = ctxGetLine(&g_ed.saveDialogCtx, 0);
    Str lineBuf = { 0 };
    renderLine_(
        &path,
        g_ed.saveDialogCtx.win.w,
        g_ed.saveDialogCtx.win.x,
        &lineBuf
    );
    screenWrite(
        &g_ed.screen,
        g_ed.strings.savePrompt.len,
        g_ed.screen.h - 1,
        lineBuf.buf,
        lineBuf.len
    );
}

void renderStatusBar(void) {
    const char *mode;
    switch (g_ed.mode) {
    case EditorMode_Insert:
        mode = "Insert";
        break;
    case EditorMode_Normal:
        mode = "Normal";
        break;
    case EditorMode_SaveDialog: {
        renderSaveDialog_();
        return;
    }
    default:
        assert(false);
    }

    Ctx *ctx = editorGetActiveCtx();

    screenWriteFmt(
        &g_ed.screen,
        0, g_ed.screen.h - 1,
        "%zi:%zi %s %s",
        ctx->cur.y + 1,
        ctx->cur.x + 1,
        mode,
        g_ed.fileCtx.edited ? "*" : ""
    );
}
