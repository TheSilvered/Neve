#include <assert.h>

#include "nv_context.h"
#include "nv_escapes.h"
#include "nv_render.h"

static void renderLine_(
    StrView *line,
    uint16_t termRow,
    size_t maxWidth,
    size_t offsetX
) {
    if (maxWidth == 0) {
        return;
    }

    size_t width = 0;
    size_t totWidth = offsetX + maxWidth;

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

        if (width <= offsetX) {
            continue;
        } else if (width - chWidth < offsetX) {
            // Draw a gray '<' at the start if a character is cut off
            editorDrawFmt(
                termRow,
                startCutoffFmt,
                width - offsetX - 1, ""
            );
        } else if (width > totWidth) {
            // Draw a gray '>' at the end if a character is cut off
            // If the character is a tab it can just draw a '»'
            editorDrawFmt(
                termRow,
                cp == '\t' ? tabFmt : endCutoffFmt,
                totWidth + chWidth - width - 1, ""
            );
            break;
        } else if (cp == '\t') {
            editorDrawFmt(termRow, tabFmt, chWidth - 1, "");
        } else {
            editorDraw(termRow, line->buf + i, ucdCh8CPLen(cp));
        }

        if (width == totWidth) {
            break;
        }
    }
}

static void renderMessage_(uint16_t rowIdx) {
    editorDraw(rowIdx, sLen("~"));
    StrView msg = { sLen("Neve editor prototype") };

    for (
        size_t pad = 1, tot = (g_ed.fileCtx.win.w - msg.len) >> 1;
        pad < tot;
        pad++
    ) {
        editorDraw(rowIdx, sLen(" "));
    }
    editorDraw(rowIdx, msg.buf, msg.len);
}

void renderFile(void) {
    if  (g_ed.mode == EditorMode_SaveDialog) {
        return;
    }

    for (uint16_t i = 0; i < g_ed.fileCtx.win.h; i++) {
        if (i + g_ed.fileCtx.win.y < ctxLineCount(&g_ed.fileCtx)) {
            StrView line = ctxGetLine(&g_ed.fileCtx, i + g_ed.fileCtx.win.y);
            renderLine_(&line, i, g_ed.fileCtx.win.w, g_ed.fileCtx.win.x);
        } else if (
            g_ed.fileCtx.text.bufLen == 0
            && i == g_ed.fileCtx.win.h / 2
        ) {
            renderMessage_(i);
        } else {
            editorDraw(i, sLen("~"));
        }
    }
}

void renderSaveDialog_(void) {
    editorDraw(
        g_ed.rows - 1,
        g_ed.strings.savePrompt.buf,
        g_ed.strings.savePrompt.len
    );

    StrView path = ctxGetLine(&g_ed.saveDialogCtx, 0);
    renderLine_(
        &path,
        g_ed.rows - 1,
        g_ed.saveDialogCtx.win.w,
        g_ed.saveDialogCtx.win.x
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

    editorDrawFmt(
        g_ed.rows - 1,
        "%zi:%zi %s ",
        ctx->cur.x + 1,
        ctx->cur.y + 1,
        mode
    );
    if (g_ed.fileCtx.edited) {
        editorDraw(g_ed.rows - 1, sLen("*"));
    }
}
