#include "nv_escapes.h"
#include "nv_render.h"

StrView visualSlice(
    const StrView *str,
    size_t visualStart,
    ptrdiff_t maxVisualLength,
    uint8_t tabStop,
    size_t *outStartWidth,
    size_t *outWidth
) {
    StrView slice = *str;

    size_t startWidth = 0;
    size_t width = 0;
    bool isInSlice = visualStart == 0;
    if (isInSlice && outStartWidth != NULL) {
        *outStartWidth = 0;
    }

    size_t sliceStartOffset = 0;
    UcdCP cp;

    for (
        ptrdiff_t i = strViewNext(str, -1, &cp);
        i != -1;
        i = strViewNext(str, i, &cp)
    ) {
        if (cp == '\n') {
            slice.len = i;
            break;
        }

        uint8_t cpWidth;
        if (cp != '\t') {
            cpWidth = ucdCPWidth(cp);
        } else {
            cpWidth = tabStop - ((width + startWidth) % tabStop);
        }

        if (!isInSlice && width + cpWidth >= visualStart) {
            isInSlice = true;
            startWidth = width + cpWidth;
            width = 0;
            sliceStartOffset = i + ucdCh8CPLen(cp);
            continue;
        }
        if (
            isInSlice
            && maxVisualLength >= 0
            && width
                + cpWidth
                + startWidth
                - visualStart > (size_t)maxVisualLength
        ) {
            slice.len = i;
            break;
        }
        width += cpWidth;
    }

    if (!isInSlice) {
        if (outStartWidth != NULL) {
            *outStartWidth = width;
        }
        if (outWidth != NULL) {
            *outWidth = 0;
        }
        slice.buf = NULL;
        slice.len = 0;
        return slice;
    }

    if (outWidth != NULL) {
        *outWidth = width;
    }
    if (outStartWidth != NULL) {
        *outStartWidth = startWidth;
    }

    slice.buf += sliceStartOffset;
    slice.len -= sliceStartOffset;
    return slice;
}

void renderLine_(Editor *ed, size_t lineIdx, uint16_t termRow) {
    if (ed->viewboxW == 0) {
        return;
    }

    StrView line = fileGetLine(&ed->file, lineIdx);

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
        uint8_t chWidth;
        if (cp == '\t') {
            chWidth = tabStop - (width % tabStop);
        } else {
            chWidth = ucdCPWidth(cp);
        }
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

void renderMessage_(Editor *ed, uint16_t rowIdx) {
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
    editorDrawFmt(ed, ed->rows - 1, "%zi:%zi", ed->curY + 1, ed->curX + 1);
}
