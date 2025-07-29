#include "nv_escapes.h"
#include "nv_render.h"

StrView visualSlice(
    StrView *str,
    size_t visualStart,
    ptrdiff_t maxVisualLength,
    size_t *outStartWidth,
    size_t *outWidth
) {
    StrView slice = *str;

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

        uint8_t cpWidth = ucdCPWidth(cp);

        if (!isInSlice && width + cpWidth >= visualStart) {
            isInSlice = true;
            if (outStartWidth != NULL) {
                *outStartWidth = width + cpWidth;
            }
            width = 0;
            sliceStartOffset = i + ucdCh8CPLen(cp);
            continue;
        }
        if (
            isInSlice
            && maxVisualLength >= 0
            && width + cpWidth > (size_t)maxVisualLength
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

    slice.buf += sliceStartOffset;
    slice.len -= sliceStartOffset;
    return slice;
}

void renderLine_(Editor *ed, size_t fileLine, uint16_t termRow) {
    StrView line = fileGetLine(&ed->file, fileLine);
    size_t offsetX = 0;
    size_t width = 0;

    StrView slice = visualSlice(
        &line,
        ed->viewboxX,
        ed->cols,
        &offsetX,
        &width
    );

    if (offsetX > ed->viewboxX) {
        editorDraw(ed, termRow, sLen(escSetStyle(colorBrightBlackFg)));
        for (size_t i = 0; i < offsetX - ed->viewboxX; i++) {
            editorDraw(ed, termRow, sLen("<"));
        }
        editorDraw(ed, termRow, sLen(escSetStyle(styleDefault)));
    }

    if (slice.len != 0) {
        editorDraw(ed, termRow, slice.buf, slice.len);
    }
}

void renderMessage_(Editor *ed, uint16_t rowIdx) {
    editorDraw(ed, rowIdx, sLen("~"));
    StrView msg = { sLen("Neve editor prototype") };

    for (
        size_t pad = 1, tot = (ed->cols - msg.len) >> 1;
        pad < tot;
        pad++
    ) {
        editorDraw(ed, rowIdx, sLen(" "));
    }
    editorDraw(ed, rowIdx, msg.buf, msg.len);
}

void renderFile(Editor *ed) {
    for (uint16_t i = 0; i < ed->rows; i++) {
        if (i + ed->viewboxY < fileLineCount(&ed->file)) {
            renderLine_(ed, i + ed->viewboxY, i);
        } else if (ed->file.contentLen == 0 && i == ed->rows / 2) {
            renderMessage_(ed, i);
        } else {
            editorDraw(ed, i, sLen("~"));
        }
    }
}
