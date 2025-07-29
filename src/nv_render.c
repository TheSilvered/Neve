#include "nv_escapes.h"
#include "nv_render.h"

StrView visualSlice(
    StrView *str,
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

void renderLine_(Editor *ed, size_t fileLine, uint16_t termRow) {
    StrView line = fileGetLine(&ed->file, fileLine);
    size_t offsetX = 0;
    size_t lineWidth = 0;

    StrView slice = visualSlice(
        &line,
        ed->scrollX,
        ed->viewboxW,
        ed->tabStop,
        &offsetX,
        &lineWidth
    );

    if (offsetX > ed->scrollX) {
        editorDrawFmt(
            ed,
            termRow,
            escSetStyle(colorBrightBlackFg)
            "<%*s"
            escSetStyle(styleDefault),
            offsetX - ed->scrollX - 1, "" // Padding with spaces
        );
    }
    UcdCP cp;
    size_t width = offsetX;
    for (
        ptrdiff_t i = strViewNext(&slice, -1, &cp);
        i != -1;
        i = strViewNext(&slice, i, &cp)
    ) {
        if (cp != '\t') {
            editorDraw(ed, termRow, slice.buf + i, ucdCh8CPLen(cp));
            width += ucdCPWidth(cp);
            continue;
        }
        uint8_t tabWidth = ed->tabStop - (width % ed->tabStop);
        editorDrawFmt(
            ed,
            termRow,
            escSetStyle(colorBrightBlackFg)
            "\xc2\xbb%*s" // "»%*s" in UTF8
            escSetStyle(styleDefault),
            tabWidth - 1, "" // Padding with spaces
        );
        width += tabWidth;
    }
    size_t actualWidth = lineWidth + offsetX - ed->scrollX;

    // If there are other characters too wide to display
    if (
        actualWidth < ed->viewboxW
        && slice.len + (slice.buf - line.buf) < line.len
    ) {
        // Safe since `slice` is a view into `line` and `slice.len` + the
        // number of bytes between the start of `line.buf` and `slice.buf` is
        // still smaller than the length of the line itself.
        if (slice.buf[slice.len] == '\t') {
            // Draw tabs normally
            editorDraw(
                ed,
                termRow,
                sLen(
                    escSetStyle(colorBrightBlackFg)
                    "\xc2\xbb" // "»" in UTF8
                    escSetStyle(styleDefault)
                )
            );
            actualWidth++;
        }
        if (ed->viewboxW - actualWidth >= 1) {
            editorDrawFmt(
                ed,
                termRow,
                escSetStyle(colorBrightBlackFg)
                "%*s>"
                escSetStyle(styleDefault),
                ed->viewboxW - actualWidth - 1, "" // Padding with spaces
            );
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
