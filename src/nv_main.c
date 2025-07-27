#include <assert.h>
#include <stdio.h>

#include "nv_editor.h"
#include "nv_escapes.h"
#include "nv_file.h"
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
    termWrite(escWithLen(
        escScreenClear
        escCursorShow
        escCursorShapeDefault
    ));

    editorQuit(&g_ed);
    termQuit();
}

void printLine(size_t fileLine, uint16_t termRow) {
    StrView line = fileGetLine(&g_ed.file, fileLine);
    size_t width = 0;
    bool isInViewbox = g_ed.viewboxX == 0;
    size_t viewboxOffset = 0;
    UcdCP cp;
    for (
        ptrdiff_t i = strViewNext(&line, -1, &cp);
        i != -1;
        i = strViewNext(&line, i, &cp)
    ) {
        if (cp == '\n') {
            line.len = i;
            break;
        }

        uint8_t cpWidth = ucdCPWidth(cp);

        if (!isInViewbox && width + cpWidth >= g_ed.viewboxX) {
            isInViewbox = true;
            editorDraw(
                &g_ed,
                termRow,
                escWithLen(escSetStyle(colorBrightBlackFg))
            );
            for (size_t i = 0; i < width + cpWidth - g_ed.viewboxX; i++) {
                editorDraw(&g_ed, termRow, "<", 1);
            }
            editorDraw(
                &g_ed,
                termRow,
                escWithLen(escSetStyle(styleDefault))
            );
            width = 0;
            viewboxOffset = i + ucdCh8CPLen(cp);
            continue;
        }
        if (width + cpWidth > g_ed.cols) {
            line.len = i;
            break;
        }
        width += cpWidth;
    }
    if (!isInViewbox) {
        return;
    }
    editorDraw(
        &g_ed,
        termRow,
        (const char *)(line.buf + viewboxOffset),
        line.len - viewboxOffset
    );
}

void refreshScreen(void) {
    bool rowsChanged, colsChanged;
    editorUpdateSize(&g_ed, &rowsChanged, &colsChanged);

    editorDraw(
        &g_ed, 0,
        escWithLen(
            escScreenClear
            escCursorHide
            escCursorSetPos("", "")
        )
    );

    for (uint16_t i = 0; i < g_ed.rows; i++) {
        if (i + g_ed.viewboxY < fileLineCount(&g_ed.file)) {
            printLine(i + g_ed.viewboxY, i);
        } else if (g_ed.file.contentLen == 0 && i == g_ed.rows / 2) {
            editorDraw(&g_ed, i, "~", 2);
            StrView msg = {
                (const UcdCh8 *)escWithLen("Neve editor prototype")
            };

            for (
                size_t pad = 1, tot = (g_ed.cols - msg.len) >> 1;
                pad < tot;
                pad++
            ) {
                editorDraw(&g_ed, i, " ", 1);
            }
            editorDraw(&g_ed, i, (char *)msg.buf, msg.len);
        } else {
            editorDraw(&g_ed, i, "~", 1);
        }
    }
    editorDraw(&g_ed, g_ed.rows - 1, escWithLen(escCursorShow));

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
