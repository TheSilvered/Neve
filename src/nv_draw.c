#include "nv_draw.h"
#include "nv_term.h"

void drawClear(void) {
    (void)termWrite("\033[2J\033[3J\033[H", 11);
}

void drawRows(void) {
    size_t rows = 0;
    if (!termSize(&rows, NULL)) {
        return;
    }
    for (int i = 0; i < rows; i++) {
        if (i == rows - 1) {
            (void)termWrite("~", 1);
        } else {
            termWrite("~\r\n", 3);
        }
    }
    (void)termWrite("\033[H", 3);
}
