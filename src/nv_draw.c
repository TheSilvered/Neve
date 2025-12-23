#include <math.h>
#include "nv_draw.h"
#include "nv_editor.h"
#include "nv_screen.h"
#include "nv_utils.h"

static const size_t _cSymbolLen = 3;
static const UcdCh8 _cSymbols[][3] = {
    [0x00] = { 0xe2, 0x90, 0x80 },
    [0x01] = { 0xe2, 0x90, 0x81 },
    [0x02] = { 0xe2, 0x90, 0x82 },
    [0x03] = { 0xe2, 0x90, 0x83 },
    [0x04] = { 0xe2, 0x90, 0x84 },
    [0x05] = { 0xe2, 0x90, 0x85 },
    [0x06] = { 0xe2, 0x90, 0x86 },
    [0x07] = { 0xe2, 0x90, 0x87 },
    [0x08] = { 0xe2, 0x90, 0x88 },
    [0x09] = { 0xe2, 0x90, 0x89 },
    [0x0a] = { 0xe2, 0x90, 0x8a },
    [0x0b] = { 0xe2, 0x90, 0x8b },
    [0x0c] = { 0xe2, 0x90, 0x8c },
    [0x0d] = { 0xe2, 0x90, 0x8d },
    [0x0e] = { 0xe2, 0x90, 0x8e },
    [0x0f] = { 0xe2, 0x90, 0x8f },
    [0x10] = { 0xe2, 0x90, 0x90 },
    [0x11] = { 0xe2, 0x90, 0x91 },
    [0x12] = { 0xe2, 0x90, 0x92 },
    [0x13] = { 0xe2, 0x90, 0x93 },
    [0x14] = { 0xe2, 0x90, 0x94 },
    [0x15] = { 0xe2, 0x90, 0x95 },
    [0x16] = { 0xe2, 0x90, 0x96 },
    [0x17] = { 0xe2, 0x90, 0x97 },
    [0x18] = { 0xe2, 0x90, 0x98 },
    [0x19] = { 0xe2, 0x90, 0x99 },
    [0x1a] = { 0xe2, 0x90, 0x9a },
    [0x1b] = { 0xe2, 0x90, 0x9b },
    [0x1c] = { 0xe2, 0x90, 0x9c },
    [0x1d] = { 0xe2, 0x90, 0x9d },
    [0x1e] = { 0xe2, 0x90, 0x9e },
    [0x1f] = { 0xe2, 0x90, 0x9f },
    [0x7f] = { 0xe2, 0x90, 0xa1 },
    [0x80] = { 0xe2, 0x96, 0xaf },
    [0x81] = { 0xe2, 0x96, 0xaf },
    [0x82] = { 0xe2, 0x96, 0xaf },
    [0x83] = { 0xe2, 0x96, 0xaf },
    [0x84] = { 0xe2, 0x96, 0xaf },
    [0x85] = { 0xe2, 0x96, 0xaf },
    [0x86] = { 0xe2, 0x96, 0xaf },
    [0x87] = { 0xe2, 0x96, 0xaf },
    [0x88] = { 0xe2, 0x96, 0xaf },
    [0x89] = { 0xe2, 0x96, 0xaf },
    [0x8a] = { 0xe2, 0x96, 0xaf },
    [0x8b] = { 0xe2, 0x96, 0xaf },
    [0x8c] = { 0xe2, 0x96, 0xaf },
    [0x8d] = { 0xe2, 0x96, 0xaf },
    [0x8e] = { 0xe2, 0x96, 0xaf },
    [0x8f] = { 0xe2, 0x96, 0xaf },
    [0x90] = { 0xe2, 0x96, 0xaf },
    [0x91] = { 0xe2, 0x96, 0xaf },
    [0x92] = { 0xe2, 0x96, 0xaf },
    [0x93] = { 0xe2, 0x96, 0xaf },
    [0x94] = { 0xe2, 0x96, 0xaf },
    [0x95] = { 0xe2, 0x96, 0xaf },
    [0x96] = { 0xe2, 0x96, 0xaf },
    [0x97] = { 0xe2, 0x96, 0xaf },
    [0x98] = { 0xe2, 0x96, 0xaf },
    [0x99] = { 0xe2, 0x96, 0xaf },
    [0x9a] = { 0xe2, 0x96, 0xaf },
    [0x9b] = { 0xe2, 0x96, 0xaf },
    [0x9c] = { 0xe2, 0x96, 0xaf },
    [0x9d] = { 0xe2, 0x96, 0xaf },
    [0x9e] = { 0xe2, 0x96, 0xaf },
    [0x9f] = { 0xe2, 0x96, 0xaf }
};

void drawUI(Screen *screen, const UI *ui) {
    drawBufPanel(screen, &ui->bufPanel);
    drawStatusBar(screen, &ui->statusBar);
}

static void _drawCtxLine(
    Screen *screen,
    const Ctx *ctx,
    size_t lineNo,
    Str *outBuf,
    uint16_t lineX,
    uint16_t lineY,
    uint16_t maxWidth,
    size_t scrollX
) {
    if (maxWidth == 0) {
        return;
    }

    strClear(outBuf, maxWidth);

    const char *tabFmt = "\xc2\xbb%*s"; // »%*s
    const char *startCutoffFmt = "<%*s";
    const char *endCutoffFmt = "%*s>";

    ptrdiff_t width = 0;
    UcdCP cp = -1;
    ptrdiff_t i = ctxIdxAt(ctx, lineNo, scrollX, (size_t *)&width);
    ptrdiff_t lineEnd = ctxLineEnd(ctx, lineNo);
    if (i >= lineEnd) {
        return;
    }

    width -= scrollX; // Makes calculations easier later
    // Put i to begin with the first character after scrollX
    if (width < 0) {
        i = ctxNext(ctx, i - 1, &cp);
        width += ucdCPWidth(cp, ctx->tabStop, width + scrollX);
    } else {
        i--;
    }

    if (width > 0) {
        strAppendFmt(outBuf, startCutoffFmt, width - 1, "");
        screenSetFg(
            screen,
            (ScreenColor){ .col = screenColT16(61) },
            lineX, lineY, 1
        );
    }

    for (
        i = ctxNext(ctx, i, &cp);
        i != -1 && i < lineEnd;
        i = ctxNext(ctx, i, &cp)
    ) {
        uint8_t chWidth = ucdCPWidth(cp, ctx->tabStop, width + scrollX);
        width += chWidth;

        if (width > maxWidth) {
            // Draw a gray '>' at the end if a character is cut off
            // If the character is a tab it can just draw a '»'
            strAppendFmt(
                outBuf,
                cp == '\t' ? tabFmt : endCutoffFmt,
                maxWidth - (width - chWidth) - 1, ""
            );
            screenSetFg(
                screen,
                (ScreenColor) { .col = screenColT16(61) },
                (uint16_t)(lineX + width - chWidth), lineY, 1
            );
            break;
        } else if (cp == '\t') {
            strAppendFmt(outBuf, tabFmt, chWidth - 1, "");
            screenSetFg(
                screen,
                (ScreenColor) { .col = screenColT16(61) },
                (uint16_t)(lineX + width - chWidth), lineY, chWidth
            );
        } else if (cp < 32 || (cp >= 0x7f && cp <= 0x9f)) {
            strAppendRaw(outBuf, _cSymbols[cp], _cSymbolLen);
            screenSetFg(
                screen,
                (ScreenColor) { .col = screenColT16(1) },
                (uint16_t)(lineX + width - chWidth), lineY, chWidth
            );
            screenSetBg(
                screen,
                (ScreenColor) { .col = screenColT16(62) },
                (uint16_t)(lineX + width - chWidth), lineY, chWidth
            );
        } else {
            UcdCh8 buf[4];
            size_t len = ucdCh8FromCP(cp, buf);
            StrView sv = { buf, len };
            strAppend(outBuf, &sv);
        }

        if (width == maxWidth) {
            break;
        }
    }

    screenWrite(screen, lineX, lineY, outBuf->buf, outBuf->len);
}

void _drawCtxSelection(
    Screen *screen,
    const Ctx *ctx,
    const UIBufPanel *panel,
    uint8_t numColWidth,
    size_t startIdx, size_t endIdx
) {
    size_t startLine, startCol;
    size_t endLine, endCol;

    ctxPosAt(ctx, startIdx, &startLine, &startCol);
    ctxPosAt(ctx, endIdx, &endLine, &endCol);

    if (
        endLine < panel->scrollY
        || startLine >= panel->scrollY + panel->elem.h
    ) {
        return;
    }

    ptrdiff_t absX = panel->elem.x - panel->scrollX + numColWidth;
    ptrdiff_t absY = panel->elem.y - panel->scrollY;
    ScreenStyle selStyle = {
        .bg = screenColT16(65),
        .fg = screenColT16(1),
        .style = screenStyleNoFmt
    };

    if (startLine == endLine) {
        screenSetStyle(
            screen,
            selStyle,
            (uint16_t)(startCol + absX),
            (uint16_t)(startLine + absY),
            (uint16_t)(endCol - startCol)
        );
        return;
    }

    screenSetStyle(
        screen,
        selStyle,
        (uint16_t)(startCol + absX),
        (uint16_t)(startLine + absY),
        screen->w
    );
    for (size_t line = startLine + 1; line < endLine; line++) {
        screenSetStyle(
            screen,
            selStyle,
            (uint16_t)(absX),
            (uint16_t)(line + absY),
            screen->w
        );
    }
    screenSetStyle(
        screen,
        selStyle,
        (uint16_t)(absX),
        (uint16_t)(endLine + absY),
        (uint16_t)(endCol)
    );
}

void drawBufPanel(Screen *screen, const UIBufPanel *panel) {
    BufHandle bufHandle = panel->bufHd;
    Buf *buf = bufRef(&g_ed.buffers, bufHandle);
    if (buf == NULL) {
        return;
    }
    Ctx *ctx = &buf->ctx;
    Str lineBuf = { 0 };
    size_t lineCount = ctxLineCount(ctx);
    uint8_t numColWidth = (uint8_t)log10((double)lineCount) + 2;

    size_t totLines = nvMin(lineCount - panel->scrollY, panel->elem.h);
    int16_t x = panel->elem.x;
    for (size_t i = 0; i < totLines; i++) {
        int16_t y = (int16_t)(panel->elem.y + i);
        size_t line = i + panel->scrollY;
        screenWriteFmt(
            screen,
            x, y,
            "%*zu",
            (int)(numColWidth - 1), line + 1
        );
        screenSetFg(
            screen,
            (ScreenColor) { .col = screenColT16(61) },
            x, y,
            numColWidth
        );
        _drawCtxLine(
            screen,
            ctx,
            line,
            &lineBuf,
            x + numColWidth, y,
            panel->elem.w - numColWidth, panel->scrollX
        );
    }

    strDestroy(&lineBuf);

    for (size_t i = 0; i < ctx->cursors.len; i++) {
        CtxCursor *cursor = &ctx->cursors.items[i];
        size_t line, col;
        ctxPosAt(ctx, cursor->idx, &line, &col);

        if (line < panel->scrollY || line > panel->scrollY + panel->elem.h
            || col < panel->scrollX || col > panel->scrollX + panel->elem.w
        ) {
            continue;
        }
        screenSetStyle(
            screen,
            (ScreenStyle) {
                .fg = screenColT16(1),
                .bg = screenColT16(8),
                .textFmt = screenFmtUnderline,
            },
            (uint16_t)(col - panel->scrollX + numColWidth),
            (uint16_t)(line - panel->scrollY),
            1
        );
        if (!ctxSelIsActive(ctx)) {
            continue;
        }
        size_t selStart = nvMin(cursor->idx, cursor->_selStart);
        size_t selEnd = nvMax(cursor->idx, cursor->_selStart);
        _drawCtxSelection(screen, ctx, panel, numColWidth, selStart, selEnd);
    }

    for (size_t i = 0; i < ctx->_sels.len; i++) {
        CtxSelection *sel = &ctx->_sels.items[i];
        _drawCtxSelection(
            screen,
            ctx,
            panel,
            numColWidth,
            sel->startIdx,
            sel->endIdx
        );
    }
}

void drawStatusBar(Screen *screen, const UIElement *statusBar) {
    screenSetTextFmt(
        screen,
        screenFmtInverse,
        statusBar->x,
        statusBar->y,
        statusBar->w
    );
    const char *modeStr = NULL;
    switch (g_ed.ui.bufPanel.mode) {
    case UIBufMode_Normal:
        modeStr = "Normal";
        break;
    case UIBufMode_Edit:
        modeStr = "Edit";
        break;
    case UIBufMode_Selection:
        modeStr = "Selection";
        break;
    default:
        nvUnreachable;
    }

    screenWriteFmt(
        screen,
        statusBar->x, 
        statusBar->y,
        "[%s]", modeStr
    );
}
