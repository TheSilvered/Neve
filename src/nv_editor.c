#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

#include "nv_editor.h"
#include "nv_escapes.h"
#include "nv_file.h"
#include "nv_mem.h"
#include "nv_term.h"

Editor g_ed = { 0 };

void editorInit(void) {
    screenInit(&g_ed.screen);
    g_ed.tabStop = 8;
    g_ed.mode = EditorMode_Normal;
    g_ed.running = true;
    ctxInitNewFile(&g_ed.fileCtx, NULL);
    ctxInitLine(&g_ed.saveDialogCtx);

    strInitFromC(&g_ed.strings.savePrompt, "File path: ");

    termWrite(sLen(escCursorShapeStillBar));
}

void editorQuit(void) {
    screenDestroy(&g_ed.screen);
    ctxDestroy(&g_ed.fileCtx);
    ctxDestroy(&g_ed.saveDialogCtx);
}

bool editorUpdateSize(void) {
    uint16_t rows, cols;
    if (!termSize(&rows, &cols)) {
        return false;
    }

    screenResize(&g_ed.screen, cols, rows);
    ctxSetWinSize(&g_ed.fileCtx, cols, rows - 1);

    if (g_ed.mode == EditorMode_SaveDialog) {
        // TODO: use visual width of savePrompt
        uint16_t saveDialogWidth = cols - g_ed.strings.savePrompt.len;
        ctxSetWinSize(&g_ed.saveDialogCtx, saveDialogWidth, 1);
        g_ed.saveDialogCtx.win.termX = g_ed.strings.savePrompt.len;
        g_ed.saveDialogCtx.win.termY = rows - 1;
    }

    return true;
}

static void handleKeyNormalMode(int32_t key) {
    Ctx *ctx = &g_ed.fileCtx;
    switch (key) {
    case TermKey_CtrlC:
        g_ed.running = false;
        return;
    case 'i':
        ctxMoveCurY(ctx, -1);
        return;
    case 'k':
        ctxMoveCurY(ctx, 1);
        return;
    case 'j':
        ctxMoveCurX(ctx, -1);
        return;
    case 'l':
        ctxMoveCurX(ctx, 1);
        return;
    case 'a':
        g_ed.mode = EditorMode_Insert;
        return;
    case 'W':
        g_ed.mode = EditorMode_SaveDialog;
        return;
    case 'w':
        if (g_ed.fileCtx.path.len == 0) {
            g_ed.mode = EditorMode_SaveDialog;
        } else {
            editorSaveFile();
        }
        return;
    default:
        return;
    }
}

static void handleInsertionKeys(int32_t key) {
    Ctx *ctx = editorGetActiveCtx();
    switch (key) {
    case TermKey_Backspace: {
        ctxRemoveBack(ctx);
        return;
    }
    case TermKey_Delete: {
        ctxRemoveForeward(ctx);
        return;
    }
    case '\r':
        key = '\n';
    default:
        ctxInsertCP(ctx, key);
    }
}

static void handleKeyInsertMode(int32_t key) {
    Ctx *ctx = &g_ed.fileCtx;
    switch (key) {
    case TermKey_CtrlC:
    case TermKey_Escape:
        g_ed.mode = EditorMode_Normal;
        return;
    default:
        handleInsertionKeys(key);
    }
}

static void handleKeySaveDialogMode(int32_t key) {
    Ctx *ctx = &g_ed.saveDialogCtx;
    switch (key) {
    case TermKey_Enter: {
        StrView path = ctxGetLine(ctx, 0);
        if (path.len != 0) {
            ctxSetPath(&g_ed.fileCtx, &path);
            editorSaveFile();
            g_ed.mode = EditorMode_Normal;
        }
        return;
    }
    case TermKey_Escape:
    case TermKey_CtrlC:
        g_ed.mode = EditorMode_Normal;
        return;
    case '\n':
        return;
    default:
        handleInsertionKeys(key);
    }
}

void editorHandleKey(uint32_t key) {
    Ctx *ctx = editorGetActiveCtx();

    switch (key) {
    case TermKey_ArrowUp:
        ctxMoveCurY(ctx, -1);
        return;
    case TermKey_ArrowDown:
        ctxMoveCurY(ctx, 1);
        return;
    case TermKey_ArrowLeft:
        ctxMoveCurX(ctx, -1);
        return;
    case TermKey_ArrowRight:
        ctxMoveCurX(ctx, 1);
        return;
    default:
        switch (g_ed.mode) {
        case EditorMode_Normal:
            handleKeyNormalMode(key);
            return;
        case EditorMode_Insert:
            handleKeyInsertMode(key);
            return;
        case EditorMode_SaveDialog:
            handleKeySaveDialogMode(key);
            return;
        }
    }
    assert(false);
}

bool editorRefresh(void) {
    if (!editorUpdateSize()) {
        return false;
    }

    renderFile();
    renderStatusBar();

    return screenRefresh(&g_ed.screen);
}

Ctx *editorGetActiveCtx(void) {
    if (g_ed.mode == EditorMode_SaveDialog) {
        return &g_ed.saveDialogCtx;
    }
    return &g_ed.fileCtx;
}

bool editorSaveFile(void) {
    if (g_ed.fileCtx.path.len == 0) {
        return false;
    }
    File file;
    FileIOResult result = fileOpen(
        &file,
        strAsC(&g_ed.fileCtx.path),
        FileMode_Write
    );
    if (result != FileIOResult_Success) {
        return false;
    }
    if (!ctxWriteToFile(&g_ed.fileCtx, &file)) {
        fileClose(&file);
        return false;
    }
    fileClose(&file);
    return true;
}
