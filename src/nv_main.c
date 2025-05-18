#include "nv_term.h"
#include "nv_draw.h"
#include <stdlib.h> // atexit
#include <ctype.h> // iscntrl
#include <stdio.h>

#define CTRL(key) ((key) & 0x1f)

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
    drawClear();
    drawRows();
}

int getKey(void) {
    int key = termGetKey();
    while (key == 0) {
        key = termGetKey();
    }
    return key;
}

int main(void) {
    if (!initTerminal())
        return false;

    for (;;) {
        refreshScreen();
        int key = getKey();
        if (key < 0) {
            termLogError("failed to read the key");
            return 1;
        }
        if (key == TermKey_CtrlC) {
            drawClear();
            break;
        }
    }
    return 0;
}
