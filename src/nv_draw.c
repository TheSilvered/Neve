#include "nv_draw.h"
#include "nv_term.h"

#include <stdio.h>

bool drawClear(Str *screenBuf) {
    const StrView clrSeq = {
        .buf = "\033[2J\033[3J\033[H",
        .len = 11
    };
    return strAppend(screenBuf, &clrSeq);
}

bool drawRows(Str *screenBuf) {
    size_t rows;
    if (!termSize(&rows, NULL)) {
        return false;
    }

    const StrView hideCursor = strViewMakeFromC("\x1b[?25l");
    const StrView showCursorAndGoHome = strViewMakeFromC("\x1b[?25h\033[H");
    const StrView tilde = strViewMakeFromC("~");
    const StrView tildeLF = strViewMakeFromC("~\r\n");

    if (!strAppend(screenBuf, &hideCursor)) {
        return false;
    }
    for (int i = 0; i < rows; i++) {
        if (i == rows - 1) {
            if (!strAppend(screenBuf, &tilde)) {
                return false;
            }
        } else {
            if (!strAppend(screenBuf, &tildeLF)) {
                return false;
            }
        }
    }
    if (!strAppend(screenBuf, &showCursorAndGoHome)) {
        return false;
    }
    return true;
}
