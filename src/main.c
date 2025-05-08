#include "sterm.h"
#include <stdlib.h> // atexit
#include <ctype.h> // iscntrl
#include <stdio.h>

int main(void) {
    if (!termInit()) {
        termLogError("failed to initialize the terminal");
        return 1;
    }

    if (atexit(termQuit) != 0) {
        perror("failed to configure exit function");
        return false;
    }

    if (!termEnableRawMode()) {
        termLogError("failed to enable raw mode");
        return 1;
    }

    for (;;) {
        TermKey key = termGetKey();
        if (termKeyErr(key)) {
            termLogError("failed to read the key");
            return 1;
        }
        if (iscntrl(key)) {
            printf("%d\r\n", key);
        } else {
            printf("%d ('%c')\r\n", key, key);
        }
        if (key == 'q') {
            break;
        }
    }
    return 0;
}
