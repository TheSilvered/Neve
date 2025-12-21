#include <math.h>
#include "nv_draw.h"
#include "nv_editor.h"
#include "nv_screen.h"
#include "nv_utils.h"

void drawUI(Screen *screen, const UI *ui) {
    drawBufPanel(screen, &ui->bufPanel);
    drawStatusBar(screen, &ui->statusBar);
}

static void _drawCtxLine(
    Screen *screen,
    const Ctx *ctx,
    size_t lineIdx,
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

    ptrdiff_t width = -(ptrdiff_t)scrollX;

    const char *tabFmt = "\xc2\xbb%*s"; // »%*s
    const char *startCutoffFmt = "<%*s";
    const char *endCutoffFmt = "%*s>";

    UcdCP cp = -1;
    for (
        ptrdiff_t i = ctxLineNextStart(ctx, lineIdx, &cp);
        i != -1;
        i = ctxLineNext(ctx, i, &cp)
    ) {
        uint8_t chWidth = ucdCPWidth(cp, ctx->tabStop, width + scrollX);
        width += chWidth;

        if (width <= 0) {
            continue;
        } else if (width - chWidth < 0) {
            // Draw a gray '<' at the start if a character is cut off
            strAppendFmt(outBuf, startCutoffFmt, width - 1, "");
            screenSetFg(
                screen,
                (ScreenColor){ .col = screenColT16(61) },
                lineX, lineY, 1
            );
        } else if ((size_t)width > maxWidth) {
            // Draw a gray '>' at the end if a character is cut off
            // If the character is a tab it can just draw a '»'
            strAppendFmt(
                outBuf,
                cp == '\t' ? tabFmt : endCutoffFmt,
                maxWidth + chWidth - width - 1, ""
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
                (uint16_t)(lineX + width - chWidth), lineY, 1
            );
        } else {
            UcdCh8 buf[4];
            size_t len = ucdCh8FromCP(cp, buf);
            StrView sv = { buf, len };
            strAppend(outBuf, &sv);
        }

        if ((size_t)width == maxWidth) {
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
