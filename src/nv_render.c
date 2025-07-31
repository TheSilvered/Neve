#include <assert.h>

#include "nv_escapes.h"
#include "nv_render.h"

static void renderLine_(Editor *ed, size_t lineIdx, uint16_t termRow) {
    if (ed->viewboxW == 0) {
        return;
    }

    StrView line = fileLine(&ed->file, lineIdx);

    size_t width = 0;
    size_t totWidth = ed->scrollX + ed->viewboxW;
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
        ptrdiff_t i = strViewNext(&line, -1, &cp);
        i != -1;
        i = strViewNext(&line, i, &cp)
    ) {
        uint8_t chWidth = ucdCPWidth(cp, tabStop, width);
        width += chWidth;

        if (width <= ed->scrollX) {
            continue;
        } else if (width - chWidth < ed->scrollX) {
            // Draw a gray '<' at the start if a character is cut off
            editorDrawFmt(
                ed,
                termRow,
                startCutoffFmt,
                width - ed->scrollX - 1, ""
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
            editorDraw(ed, termRow, line.buf + i, ucdCh8CPLen(cp));
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
    for (uint16_t i = 0; i < ed->viewboxH; i++) {
        if (i + ed->scrollY < fileLineCount(&ed->file)) {
            renderLine_(ed, i + ed->scrollY, i);
        } else if (ed->file.contentLen == 0 && i == ed->viewboxH / 2) {
            renderMessage_(ed, i);
        } else {
            editorDraw(ed, i, sLen("~"));
        }
    }
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
