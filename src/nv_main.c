#include <stdlib.h>
#include <stdio.h>
#include "nv_term.h"
#include "nv_editor.h"
#include "nv_escapes.h"

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
    editorDrawBegin(&g_ed);

    if (editorRowsChanged(&g_ed)) {
        editorDraw(
            &g_ed,
            escWithLen(
                escCursorHide
                escScreenClear
                escCursorSetPos("", "")
            )
        );
    }

    for (size_t i = 0; i < g_ed.rows; i++) {
        // clear line by line if the number of rows remains the same
        if (!editorRowsChanged(&g_ed)) {
            editorDraw(&g_ed, escWithLen(escLineClear));
        }

        if (i == g_ed.rows / 2) {
            editorDraw(&g_ed, "~", 2);
            StrView msg = {
                (const UcdCh8 *)escWithLen("Neve editor prototype")
            };

            for (
                size_t pad = 1, tot = (g_ed.cols - msg.len) >> 1;
                pad < tot;
                pad++
            ) {
                editorDraw(&g_ed, " ", 1);
            }
            editorDraw(&g_ed, (char *)msg.buf, msg.len);
            editorDraw(&g_ed, "\r\n", 2);

        } else if (i == g_ed.rows - 1) {
            editorDraw(&g_ed, "~", 1);
        } else {
            editorDraw(&g_ed, "~\r\n", 3);
        }
    }
    editorDraw(&g_ed, escWithLen(escCursorShow escCursorSetPos("", "")));

    editorDrawEnd(&g_ed);
}

int main(void) {
    if (!initNeve()) {
        return 1;
    }

    for (;;) {
        refreshScreen();
        int key = termGetKey();
        if (key < 0) {
            termLogError("failed to read the key");
            return 1;
        }
        if (key == TermKey_CtrlC) {
            break;
        }
    }
    return 0;
}
