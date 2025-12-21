#include <stdarg.h>
#include <assert.h>
#include <math.h>

#include "nv_buffer.h"
#include "nv_draw.h"
#include "nv_editor.h"
#include "nv_escapes.h"
#include "nv_logging.h"
#include "nv_screen.h"
#include "nv_term.h"
#include "nv_time.h"
#include "nv_tui.h"

Editor g_ed = { 0 };

void editorInit(void) {
    screenInit(&g_ed.screen);
    bufMapInit(&g_ed.buffers);
    g_ed.running = true;
    g_ed.lastUpdate = 0;

    uiInit(&g_ed.ui);

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
    uiResize(&g_ed.ui, cols, rows);

    return true;
}

void editorHandleKey(int32_t key) {
    if (uiHandleKey(&g_ed.ui.elem, key)) {
        return;
    }

    if (key == TermKey_CtrlQ) {
        g_ed.running = false;
    }
}

bool editorRefresh(void) {
    uint64_t time = timeRelNs();
    if (time - g_ed.lastUpdate < 16000000) {
        timeSleep(16000000 + g_ed.lastUpdate - time);
    }
    g_ed.lastUpdate = timeRelNs();

    if (!editorUpdateSize()) {
        return false;
    }
    uiUpdate(&g_ed.ui.elem);

    screenClear(&g_ed.screen, -1);
    drawUI(&g_ed.screen, &g_ed.ui);

    return screenRefresh(&g_ed.screen);
}

bool editorOpen(const char *path) {
    File file;
    BufHandle newBuf = bufInvalidHandle;
    bool success = true;
    switch (fileOpen(&file, path, FileMode_Read)) {
    case FileIOResult_Success:
        bufClose(&g_ed.buffers, g_ed.ui.bufPanel.bufHd);
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
        bufClose(&g_ed.buffers, g_ed.ui.bufPanel.bufHd);
        newBuf = bufInitEmpty(&g_ed.buffers);
        bufSetPath(&g_ed.buffers, newBuf, path);
        break;
    default:
        success = false;
    }
    g_ed.ui.bufPanel.bufHd = newBuf;
    fileClose(&file);
    return success;
}

void editorNewBuf(void) {
    bufClose(&g_ed.buffers, g_ed.ui.bufPanel.bufHd);
    BufHandle newBuf = bufInitEmpty(&g_ed.buffers);
    g_ed.ui.bufPanel.bufHd = newBuf;
}
