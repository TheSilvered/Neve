#include <assert.h>

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
        size_t pad = 1, tot = (ed->viewboxW - msg.len) >> 1;
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

    for (uint16_t i = 0; i < ed->viewboxH; i++) {
        if (i + ed->scrollY < fileLineCount(&ed->file)) {
            StrView line = fileLine(&ed->file, i + ed->scrollY);
            renderLine_(ed, &line, i, ed->viewboxW, ed->scrollX);
        } else if (ed->file.contentLen == 0 && i == ed->viewboxH / 2) {
            renderMessage_(ed, i);
        } else {
            editorDraw(ed, i, sLen("~"));
        }
    }
}

void renderSaveDialog_(Editor *ed) {
    const char msg[] = "File Name: ";
    editorDraw(ed, ed->rows - 1, (const UcdCh8 *)msg, sizeof(msg));
    UcdCP cp;
    size_t width = 0;
    for (
        ptrdiff_t i = strViewNext((StrView *)&ed->file.path, -1, &cp);
        i != -1;
        i = strViewNext((StrView *)&ed->file.path, i, &cp)
    ) {
        width += ucdCPWidth(cp, ed->tabStop, width);
    }

    size_t maxWidth = ed->cols - sizeof(msg);
    renderLine_(
        ed,
        (StrView *)&ed->file.path,
        ed->rows - 1,
        maxWidth,
        maxWidth >= width ? 0 : width - maxWidth
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

    editorDrawFmt(
        ed,
        ed->rows - 1,
        "%zi:%zi %s",
        ed->curY + 1,
        ed->curX + 1,
        mode
    );
}
