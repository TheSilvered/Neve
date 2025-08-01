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

void refreshScreen(void) {
    editorUpdateSize(&g_ed);
    // Leave the last row for a status bar.
    editorSetViewboxSize(&g_ed, g_ed.cols, g_ed.rows - 1);

    editorDraw(
        &g_ed, 0,
        sLen(
            escScreenClear
            escCursorHide
            escCursorSetPos("", "")
        )
    );

    renderFile(&g_ed);
    renderStatusBar(&g_ed);

    if (g_ed.mode != EditorMode_SaveDialog) {
        editorDraw(&g_ed, g_ed.rows - 1, sLen(escCursorShow));
    }
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

static size_t keyToUTF8(int32_t key, bool allowLF, UcdCh8 *outBuf) {
    if (key < 0 || key > UcdCPMax) {
        return 0;
    }

    UdbCPInfo info = udbGetCPInfo((UcdCP)key);
    // Do not insert control characters
    if (
        (key != '\n' || !allowLF)
        && info.category >= UdbCategory_C_First
        && info.category <= UdbCategory_C_Last
    ) {
        return 0;
    }

    return ucdCh8FromCP((UcdCP)key, outBuf);
}

void handleKeyNormalMode(int32_t key) {
    switch (key) {
    case TermKey_CtrlC:
        g_ed.running = false;
        return;
    case TermKey_ArrowUp:
    case 'i':
        editorMoveCursorY(&g_ed, -1);
        return;
    case TermKey_ArrowDown:
    case 'k':
        editorMoveCursorY(&g_ed, 1);
        return;
    case TermKey_ArrowLeft:
    case 'j':
        editorMoveCursorX(&g_ed, -1);
        return;
    case TermKey_ArrowRight:
    case 'l':
        editorMoveCursorX(&g_ed, 1);
        return;
    case 'a':
        g_ed.mode = EditorMode_Insert;
        return;
    case 'W':
        g_ed.mode = EditorMode_SaveDialog;
        return;
    case 'w':
        if (g_ed.file.path.len == 0) {
            g_ed.mode = EditorMode_SaveDialog;
            return;
        }
        fileSave(&g_ed.file);
        return;
    default:
        return;
    }
}

void handleKeyInsertMode(int32_t key) {
    switch (key) {
    case TermKey_CtrlC:
        g_ed.running = false;
        return;
    case TermKey_ArrowUp:
        editorMoveCursorY(&g_ed, -1);
        return;
    case TermKey_ArrowDown:
        editorMoveCursorY(&g_ed, 1);
        return;
    case TermKey_ArrowLeft:
        editorMoveCursorX(&g_ed, -1);
        return;
    case TermKey_ArrowRight:
        editorMoveCursorX(&g_ed, 1);
        return;
    case TermKey_Escape:
        g_ed.mode = EditorMode_Normal;
        return;
    case TermKey_Backspace: {
        if (g_ed.fileCurIdx == 0) {
            return;
        }
        StrView content = fileContent(&g_ed.file);
        size_t startIdx = strViewPrev(&content, g_ed.fileCurIdx, NULL);
        size_t endIdx = g_ed.fileCurIdx;
        // Edit content only _after_ the cursor otherwise it could end up in
        // the middle of a multibyte sequence.
        editorMoveCursorIdx(&g_ed, -1);
        fileRemove(&g_ed.file, startIdx, endIdx);
        return;
    }
    case '\r':
        key = '\n';
        break;
    default:
        break;
    }

    UcdCh8 buf[4];
    size_t len = keyToUTF8(key, true, buf);
    if (len == 0) {
        return;
    }
    // Edit content only _after_ the cursor otherwise it could end up in
    // the middle of a multibyte sequence.
    fileInsert(&g_ed.file, g_ed.fileCurIdx, buf, len);
    editorMoveCursorIdx(&g_ed, 1);
}

void handleKeySaveDialogMode(int32_t key) {
    switch (key) {
    case TermKey_Enter:
        if (g_ed.file.path.len != 0) {
            fileSave(&g_ed.file);
            g_ed.mode = EditorMode_Normal;
        }
        return;
    case TermKey_Escape:
    case TermKey_CtrlC:
        g_ed.mode = EditorMode_Normal;
        return;
    case TermKey_Backspace:
        strPop(&g_ed.file.path, 1);
        return;
    default:
        break;
    }

    UcdCh8 buf[4];
    size_t len = keyToUTF8(key, false, buf);
    if (len == 0) {
        return;
    }
    StrView ch = {
        .buf = buf,
        .len = len
    };

    strAppend(&g_ed.file.path, &ch);
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
