#include <stdlib.h>
#include <stdio.h>
#include "nv_term.h"
#include "nv_draw.h"

static Str g_screenBuf = { 0 };

bool initTerminal(void) {
    if (!termInit()) {
        termLogError("failed to initialize the terminal");
        return false;
    }

    if (atexit(termQuit) != 0) {
        perror("failed to configure exit function");
        return false;
    }

    if (!termEnableRawMode(1)) {
        termLogError("failed to enable raw mode");
        return false;
    }

    return true;
}

void refreshScreen(void) {
    if (!strClear(&g_screenBuf, 1024)) {
        return;
    }

    drawClear(&g_screenBuf);
    drawRows(&g_screenBuf);

    termWrite(g_screenBuf.buf, g_screenBuf.len);
}

void clearScreen(void) {
    strClear(&g_screenBuf, 10);
    drawClear(&g_screenBuf);
    termWrite(g_screenBuf.buf, g_screenBuf.len);
}

int getKey(void) {
    int key = termGetKey();
    while (key == 0) {
        key = termGetKey();
    }
    return key;
}

int main(void) {
    if (!initTerminal()) {
        return 1;
    }

    for (;;) {
        refreshScreen();
        int key = getKey();
        if (key < 0) {
            termLogError("failed to read the key");
            return 1;
        }
        if (key == TermKey_CtrlC) {
            clearScreen();
            break;
        }
    }
    return 0;
}
