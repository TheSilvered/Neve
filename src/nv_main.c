#include "nv_term.h"
#include <stdlib.h> // atexit
#include <ctype.h> // iscntrl
#include <stdio.h>

#define CTRL(key) ((key) & 0x1f)

int main(void) {
    if (!termInit()) {
        termLogError("failed to initialize the terminal");
        return 1;
    }

    if (atexit(termQuit) != 0) {
        perror("failed to configure exit function");
        return false;
    }

    if (!termEnableRawMode(0)) {
        termLogError("failed to enable raw mode");
        return 1;
    }

    termClearScreen();

    for (;;) {
        TermKey key = termGetKey();
        if (termKeyErr(key)) {
            termLogError("failed to read the key");
            return 1;
        }
        printf("key = %d\r\n", key);
        if (key == CTRL('c')) {
            termClearScreen();
            break;
        }
    }
    return 0;
}
