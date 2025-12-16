#include "nv_draw.h"
#include "nv_editor.h"
#include "nv_utils.h"

static void _drawCtxLine(
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

    ptrdiff_t width = -scrollX;

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
                &g_ed.screen,
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
                &g_ed.screen,
                (ScreenColor) { .col = screenColT16(61) },
                lineX + width - chWidth, lineY, 1
            );
            break;
        } else if (cp == '\t') {
            strAppendFmt(outBuf, tabFmt, chWidth - 1, "");
            screenSetFg(
                &g_ed.screen,
                (ScreenColor) { .col = screenColT16(61) },
                lineX + width - chWidth, lineY, 1
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

    screenWrite(&g_ed.screen, lineX, lineY, outBuf->buf, outBuf->len);
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

    size_t totLines = nvMin(lineCount - panel->y, panel->h);
    for (size_t i = 0; i < totLines; i++) {
        _drawCtxLine(
            ctx,
            i + panel->scrollY,
            &lineBuf,
            panel->x, i,
            panel->w, panel->scrollX
        );
    }
}
