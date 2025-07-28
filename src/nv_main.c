#include <assert.h>
#include <stdio.h>

#include "nv_editor.h"
#include "nv_escapes.h"
#include "nv_file.h"
#include "nv_render.h"
#include "nv_term.h"

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

void refreshScreen(void) {
    bool rowsChanged, colsChanged;
    editorUpdateSize(&g_ed, &rowsChanged, &colsChanged);

    editorDraw(
        &g_ed, 0,
        sLen(
            escScreenClear
            escCursorHide
            escCursorSetPos("", "")
        )
    );

    renderFile(&g_ed);

    editorDraw(&g_ed, g_ed.rows - 1, sLen(escCursorShow));
    editorDrawEnd(&g_ed);
}

void loadOrCreateFile(const char *path) {
    // Stop execution if out of memory.
    // Set the path of the empty file to `path` if the file does not exist.
    // Otherwise keep the empty file (on failure) or the loaded file.
    switch (fileInitOpen(&g_ed.file, path)) {
    case FileIOResult_FileNotFound:
        strInitFromC(&g_ed.file.path, path);
    default:
        return;
    }
}

void handleKey(int32_t key) {
    switch (key) {
    case TermKey_CtrlC:
        g_ed.running = false;
        break;
    case TermKey_ArrowUp:
    case 'i':
        editorMoveCursor(&g_ed, 0, -1);
        break;
    case TermKey_ArrowDown:
    case 'k':
        editorMoveCursor(&g_ed, 0, 1);
        break;
    case TermKey_ArrowLeft:
    case 'j':
        editorMoveCursor(&g_ed, -1, 0);
        break;
    case TermKey_ArrowRight:
    case 'l':
        editorMoveCursor(&g_ed, 1, 0);
        break;
    default:
        break;
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
        refreshScreen();
        int32_t key = termGetKey();
        if (key < 0) {
            termLogError("failed to read the key");
            return 1;
        }
        handleKey(key);
    }

    quitNeve();
    return 0;
}
