#include <assert.h>

#include "nv_context.h"
#include "nv_escapes.h"
#include "nv_render.h"

static void renderLine_(
    Editor *ed,
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
    uint8_t tabStop = ed->tabStop;

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
        uint8_t chWidth = ucdCPWidth(cp, tabStop, width);
        width += chWidth;

        if (width <= offsetX) {
            continue;
        } else if (width - chWidth < offsetX) {
            // Draw a gray '<' at the start if a character is cut off
            editorDrawFmt(
                ed,
                termRow,
                startCutoffFmt,
                width - offsetX - 1, ""
            );
        } else if (width > totWidth) {
            // Draw a gray '>' at the end if a character is cut off
            // If the character is a tab it can just draw a '»'
            editorDrawFmt(
                ed,
                termRow,
                cp == '\t' ? tabFmt : endCutoffFmt,
                totWidth + chWidth - width - 1, ""
            );
            break;
        } else if (cp == '\t') {
            editorDrawFmt(ed, termRow, tabFmt, chWidth - 1, "");
        } else {
            editorDraw(ed, termRow, line->buf + i, ucdCh8CPLen(cp));
        }

        if (width == totWidth) {
            break;
        }
    }
}

static void renderMessage_(Editor *ed, uint16_t rowIdx) {
    editorDraw(ed, rowIdx, sLen("~"));
    StrView msg = { sLen("Neve editor prototype") };

    for (
        size_t pad = 1, tot = (ed->fileCtx.win.w - msg.len) >> 1;
        pad < tot;
        pad++
    ) {
        editorDraw(ed, rowIdx, sLen(" "));
    }
    editorDraw(ed, rowIdx, msg.buf, msg.len);
}

void renderFile(Editor *ed) {
    if  (ed->mode == EditorMode_SaveDialog) {
        return;
    }

    for (uint16_t i = 0; i < ed->fileCtx.win.h; i++) {
        if (i + ed->fileCtx.win.y < ctxLineCount(&ed->fileCtx)) {
            StrView line = ctxGetLine(&ed->fileCtx, i + ed->fileCtx.win.y);
            renderLine_(ed, &line, i, ed->fileCtx.win.w, ed->fileCtx.win.x);
        } else if (ed->fileCtx.text.bufLen == 0 && i == ed->fileCtx.win.h / 2) {
            renderMessage_(ed, i);
        } else {
            editorDraw(ed, i, sLen("~"));
        }
    }
}

void renderSaveDialog_(Editor *ed) {
    editorDraw(
        ed,
        ed->rows - 1,
        ed->strings.savePrompt.buf,
        ed->strings.savePrompt.len
    );

    StrView path = ctxGetLine(&ed->saveDialogCtx, 0);
    renderLine_(
        ed,
        &path,
        ed->rows - 1,
        ed->saveDialogCtx.win.w,
        ed->saveDialogCtx.win.x
    );
}

void renderStatusBar(Editor *ed) {
    const char *mode;
    switch (ed->mode) {
    case EditorMode_Insert:
        mode = "Insert";
        break;
    case EditorMode_Normal:
        mode = "Normal";
        break;
    case EditorMode_SaveDialog: {
        renderSaveDialog_(ed);
        return;
    }
    default:
        assert(false);
    }

    Ctx *ctx = editorGetActiveCtx(ed);

    editorDrawFmt(
        ed,
        ed->rows - 1,
        "%zi:%zi %s ",
        ctx->cur.x + 1,
        ctx->cur.y + 1,
        mode
    );
    if (ed->fileCtx.edited) {
        editorDraw(ed, ed->rows - 1, sLen("*"));
    }
}
