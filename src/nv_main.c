#include <assert.h>
#include <stdio.h>

#include "nv_editor.h"
#include "nv_escapes.h"
#include "nv_file.h"
#include "nv_render.h"
#include "nv_term.h"
#include "nv_udb.h"

bool initNeve(void) {
    if (!termInit()) {
        termLogError("failed to initialize the terminal");
        return false;
    }

    if (!termEnableRawMode(1)) {
        termLogError("failed to enable raw mode");
        return false;
    }

    editorInit(&g_ed);
    return true;
}

void quitNeve(void) {
    termWrite(sLen(
        escScreenClear
        escCursorShow
        escCursorShapeDefault
    ));

    editorQuit(&g_ed);
    termQuit();
}

void loadOrCreateFile(const char *path) {
    File file;
    switch (fileOpen(&file, path, FileMode_Read)) {
    case FileIOResult_FileNotFound:
        ctxDestroy(&g_ed.fileCtx);
        ctxInitNewFile(&g_ed.fileCtx, path);
        return;
    case FileIOResult_Success:
        ctxDestroy(&g_ed.fileCtx);
        ctxInitFromFile(&g_ed.fileCtx, &file);
        fileClose(&file);
        return;
    default:
        return;
    }
}

void handleKeyNormalMode(int32_t key) {
    Ctx *ctx = &g_ed.fileCtx;
    switch (key) {
    case TermKey_CtrlC:
        g_ed.running = false;
        return;
    case TermKey_ArrowUp:
    case 'i':
        ctxMoveCurY(ctx, -1);
        return;
    case TermKey_ArrowDown:
    case 'k':
        ctxMoveCurY(ctx, 1);
        return;
    case TermKey_ArrowLeft:
    case 'j':
        ctxMoveCurX(ctx, -1);
        return;
    case TermKey_ArrowRight:
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
            editorSaveFile(&g_ed);
        }
        return;
    default:
        return;
    }
}

void handleKeyInsertMode(int32_t key) {
    Ctx *ctx = &g_ed.fileCtx;
    switch (key) {
    case TermKey_CtrlC:
        g_ed.running = false;
        return;
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
    case TermKey_Escape:
        g_ed.mode = EditorMode_Normal;
        return;
    case TermKey_Backspace: {
        ctxRemoveBack(ctx);
        return;
    }
    case '\r':
        key = '\n';
    default:
        ctxInsertCP(ctx, key);
    }
}

void handleKeySaveDialogMode(int32_t key) {
    Ctx *ctx = &g_ed.saveDialogCtx;
    switch (key) {
    case TermKey_Enter: {
        StrView path = ctxGetLine(ctx, 0);
        if (path.len != 0) {
            ctxSetPath(&g_ed.fileCtx, &path);
            editorSaveFile(&g_ed);
            g_ed.mode = EditorMode_Normal;
        }
        return;
    }
    case TermKey_Escape:
    case TermKey_CtrlC:
        g_ed.mode = EditorMode_Normal;
        return;
    case TermKey_ArrowLeft:
        ctxMoveCurX(ctx, -1);
        return;
    case TermKey_ArrowRight:
        ctxMoveCurX(ctx, 1);
        return;
    case TermKey_Backspace:
        ctxRemoveBack(ctx);
        return;
    default:
        ctxInsertCP(ctx, key);
        return;
    }
}

// TODO: use wmain on Windows
int main(int argc, char **argv) {
    if (argc > 2) {
        printf("Usage: neve [file]\n");
        return 1;
    }

    if (!initNeve()) {
        return 1;
    }

    if (argc == 2) {
        loadOrCreateFile(argv[1]);
    }

    while (g_ed.running) {
        editorRefresh(&g_ed);
        int32_t key = termGetKey();
        if (key < 0) {
            termLogError("failed to read the key");
            return 1;
        }

        switch (g_ed.mode) {
        case EditorMode_Normal:
            handleKeyNormalMode(key);
            break;
        case EditorMode_Insert:
            handleKeyInsertMode(key);
            break;
        case EditorMode_SaveDialog:
            handleKeySaveDialogMode(key);
            break;
        default:
            assert(false);
        }
    }

    quitNeve();
    return 0;
}
