#include <stdlib.h>
#include <stdio.h>
#include "nv_term.h"
#include "nv_string.h"
#include "nv_escapes.h"

#define prevScreenSize 16384

static Str g_screenBuf = { 0 };

static void screenWrite(const char *buf, size_t len) {
    StrView sv = {
        .buf = (const UcdCh8 *)buf,
        .len = len
    };
    (void)strAppend(&g_screenBuf, &sv);
}

void quitTerminal(void);

bool initTerminal(void) {
    if (!termInit()) {
        termLogError("failed to initialize the terminal");
        return false;
    }

    if (atexit(quitTerminal) != 0) {
        perror("failed to configure exit function");
        return false;
    }

    if (!termEnableRawMode(1)) {
        termLogError("failed to enable raw mode");
        return false;
    }

    return true;
}

void quitTerminal(void) {
    strClear(&g_screenBuf, 10);
    screenWrite(escWithLen(escScreenClear escCursorShow escCursorShapeDefault));
    termWrite(g_screenBuf.buf, g_screenBuf.len);

    strDestroy(&g_screenBuf);
}

void refreshScreen(void) {
    size_t rows, cols;
    if (!termSize(&rows, &cols)) {
        return;
    }

    if (!strClear(&g_screenBuf, g_screenBuf.len)) {
        return;
    }

    screenWrite(escWithLen(escCursorHide escScreenClear));
    for (size_t i = 0; i < rows; i++) {
        if (i == rows - 1) {
            screenWrite("~", 1);
        } else {
            screenWrite("~\r\n", 3);
        }
    }
    screenWrite(escWithLen(escCursorShow escCursorSetPos("", "")));

    termWrite(g_screenBuf.buf, g_screenBuf.len);
}

int main(void) {
    if (!initTerminal()) {
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
