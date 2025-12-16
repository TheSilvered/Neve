#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include "nv_editor.h"
#include "nv_escapes.h"
#include "nv_screen.h"
#include "nv_term.h"
#include "nv_utils.h"

Editor g_ed = { 0 };

void editorInit(void) {
    screenInit(&g_ed.screen);
    bufMapInit(&g_ed.buffers);
    g_ed.running = true;

    strInitFromC(&g_ed.strings.savePrompt, "File path: ");
    strInitFromC(&g_ed.strings.noFilePath, "<New File>");

    termWrite(sLen(
        escEnableAltBuffer
        escCursorShapeStillBlock
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
    g_ed.bufPanel.w = cols;
    g_ed.bufPanel.h = rows;

    return true;
}

bool editorRefresh(void) {
    if (!editorUpdateSize()) {
        return false;
    }

    screenClear(&g_ed.screen, -1);
    _renderFile(&editorGetActiveBuf()->ctx);
    _renderStatusBar();

    return screenRefresh(&g_ed.screen);
}

Ctx *editorGetActiveCtx(void) {
    if (g_ed.savingFile) {
        return &g_ed.saveDialogCtx;
    }
    return &g_ed.fileBuf.ctx;
}

Buf *editorGetActiveBuf(void) {
    return &g_ed.fileBuf;
}

bool editorSaveFile(void) {
    return bufWriteToDisk(&g_ed.fileBuf);
}
