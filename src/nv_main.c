#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "nv_editor.h"
#include "nv_escapes.h"
#include "nv_file.h"
#include "nv_term.h"
#include "nv_udb.h"

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
        escCursorShow
        escCursorShapeDefault
    ));

    editorQuit(&g_ed);
    termQuit();
}

void printLine(size_t fileLine, uint16_t termRow) {
    StrView line = fileGetLine(&g_ed.file, fileLine);
    size_t visualWidth = 0;
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

        UdbCPInfo info = udbGetCPInfo(cp);
        uint8_t cpVisualWidth = 0;
        switch (info.width) {
        case UdbWidth_Fullwidth:
        case UdbWidth_Wide:
            cpVisualWidth = 2;
            break;
        case UdbWidth_Ambiguous:
        case UdbWidth_Neutral:
        case UdbWidth_Narrow:
        case UdbWidth_Halfwidth:
            cpVisualWidth = 1;
            break;
        default:
            assert(false);
        }

        if (visualWidth + cpVisualWidth > g_ed.cols) {
            line.len = i;
            break;
        }
        visualWidth += cpVisualWidth;
    }
    editorDraw(&g_ed, termRow, (const char *)line.buf, line.len);
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
        if (i < fileLineCount(&g_ed.file)) {
            printLine(i, i);
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

skipPaint:
    editorDrawEnd(&g_ed);
}

int main(void) {
    if (!initNeve()) {
        return 1;
    }

    switch (fileInitOpen(&g_ed.file, "../README.md")) {
    case FileIOResult_FileNotFound:
        printf("File not found.");
        return 1;
    case FileIOResult_OutOfMemory:
        printf("Out of memory.");
        return 1;
    case FileIOResult_Success:
        break;
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
            termWrite(escWithLen(escScreenClear));
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
