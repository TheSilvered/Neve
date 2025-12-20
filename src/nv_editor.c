#include <stdarg.h>
#include <assert.h>
#include <math.h>

#include "nv_draw.h"
#include "nv_editor.h"
#include "nv_escapes.h"
#include "nv_screen.h"
#include "nv_term.h"
#include "nv_tui.h"

Editor g_ed = { 0 };

void editorInit(void) {
    screenInit(&g_ed.screen);
    bufMapInit(&g_ed.buffers);
    g_ed.running = true;

    uiBufPanelInit(&g_ed.bufPanel, bufInvalidHandle);

    strInitFromC(&g_ed.strings.savePrompt, "File path: ");
    strInitFromC(&g_ed.strings.noFilePath, "<New File>");

    termWrite(sLen(
        escEnableAltBuffer
        escCursorHide
    ));
}

void editorQuit(void) {
    screenDestroy(&g_ed.screen);
    bufMapDestroy(&g_ed.buffers);
}

bool editorUpdateSize(void) {
    uint16_t rows, cols;
    if (!termSize(&rows, &cols)) {
        return false;
    }

    screenResize(&g_ed.screen, cols, rows);
    g_ed.bufPanel.elem.w = cols;
    g_ed.bufPanel.elem.h = rows;

    return true;
}

void editorHandleKey(int32_t key) {
    if (key == TermKey_CtrlQ) {
        g_ed.running = false;
        return;
    }
    uiHandleKey(&g_ed.bufPanel.elem, key);
}

bool editorRefresh(void) {
    if (!editorUpdateSize()) {
        return false;
    }
    uiUpdate(&g_ed.bufPanel.elem);

    screenClear(&g_ed.screen, -1);
    drawBufPanel(&g_ed.bufPanel);

    return screenRefresh(&g_ed.screen);
}

bool editorOpen(const char *path) {
    File file;
    BufHandle newBuf;
    bool success = true;
    switch (fileOpen(&file, path, FileMode_Read)) {
    case FileIOResult_Success:
        bufClose(&g_ed.buffers, g_ed.bufPanel.bufHd);
        if (
            bufInitFromFile(
                &g_ed.buffers,
                &file,
                &newBuf
            ).kind != BufResult_Success
        ) {
            success = false;
            break;
        }
        break;
    case FileIOResult_FileNotFound:
        bufClose(&g_ed.buffers, g_ed.bufPanel.bufHd);
        newBuf = bufInitEmpty(&g_ed.buffers);
        bufSetPath(&g_ed.buffers, newBuf, path);
        g_ed.bufPanel.bufHd = newBuf;
        break;
    default:
        success = false;
    }
    fileClose(&file);
    return success;
}

void editorNewBuf(void) {
    bufClose(&g_ed.buffers, g_ed.bufPanel.bufHd);
    BufHandle newBuf = bufInitEmpty(&g_ed.buffers);
    g_ed.bufPanel.bufHd = newBuf;
}
