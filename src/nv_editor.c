#include "nv_editor.h"
#include "nv_term.h"

Editor g_ed;

void editorInit(Editor *ed) {
    ed->curX = 0;
    ed->curY = 0;
    (void)strInit(&ed->screenBuf, 0); // success guaranteed with reserve=0
}

void editorQuit(Editor *ed) {
    strDestroy(&ed->screenBuf);
}

bool editorDraw(Editor *ed, const char *buf, size_t len) {
    StrView sv = {
        .buf = (const UcdCh8 *)buf,
        .len = len
    };
    return strAppend(&ed->screenBuf, &sv);
}

bool editorFlipScreen(Editor *ed) {
    if (!termWrite(ed->screenBuf.buf, ed->screenBuf.len)) {
        return false;
    }
    (void)strClear(&ed->screenBuf, ed->screenBuf.len); // success guaranteed
    return true;
}
