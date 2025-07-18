#include <stdlib.h>
#include <stdio.h>
#include "nv_term.h"
#include "nv_editor.h"
#include "nv_escapes.h"
#include "nv_file.h"

void quitNeve(void);

bool initNeve(void) {
    if (!termInit()) {
        termLogError("failed to initialize the terminal");
        return false;
    }

    if (atexit(quitNeve) != 0) {
        perror("failed to configure exit function");
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

void refreshScreen(void) {
    bool rowsChanged, colsChanged;
    editorUpdateSize(&g_ed, &rowsChanged, &colsChanged);

    if (!rowsChanged && !colsChanged) {
        goto skipPaint;
    }

    editorDraw(
        &g_ed, 0,
        escWithLen(
            escScreenClear
            escCursorHide
            escCursorSetPos("", "")
        )
    );

    for (uint16_t i = 0; i < g_ed.rows; i++) {
        if (i == g_ed.rows / 2) {
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

skipPaint:
    editorDrawEnd(&g_ed);
}

int main(void) {
    if (!initNeve()) {
        return 1;
    }

    bool running = true;
    while (running) {
        refreshScreen();
        int key = termGetKey();
        if (key < 0) {
            termLogError("failed to read the key");
            return 1;
        }
        switch (key) {
        case TermKey_CtrlC:
            running = false;
            break;
        case TermKey_ArrowUp:
            if (g_ed.curY != 0) {
                g_ed.curY--;
            }
            break;
        case TermKey_ArrowDown:
            if (g_ed.curY < g_ed.rows - 1) {
                g_ed.curY++;
            }
            break;
        case TermKey_ArrowLeft:
            if (g_ed.curX != 0) {
                g_ed.curX--;
            }
            break;
        case TermKey_ArrowRight:
            if (g_ed.curX < g_ed.cols - 1) {
                g_ed.curX++;
            }
            break;
        default:
            break;
        }
    }
    return 0;
}
