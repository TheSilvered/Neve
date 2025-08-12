#ifndef NV_RENDER_H_
#define NV_RENDER_H_

#include "nv_string.h"

// The painting surface.
typedef struct {
    uint16_t w, h;
    Str buf;
    Str *editRows;
    Str *displayRows;
    bool resized;
} Screen;

// Initialize the screen.
void screenInit(Screen *screen);
// Destroy the contents of the screen.
void screenDestroy(Screen *screen);

// Resize the screen. If the size changes the screen is cleared.
void screenResize(Screen *screen, uint16_t w, uint16_t h);

// Write a string to the screen.
void screenWrite(
    Screen *screen,
    uint16_t x, uint16_t y,
    const UcdCh8 *str, size_t len
);

// Write a string to the screen using printf-style formatting.
void screenWriteFmt(
    Screen *screen,
    uint16_t x, uint16_t y,
    const char *fmt, ...
);

// Clear a line of the screen.
// If `line == -1` all lines are cleared.
void screenClear(Screen *screen, int32_t line);

// Display any draw calls to the screen.
bool screenRefresh(Screen *screen);

void renderFile(void);
void renderStatusBar(void);

#endif // !NV_RENDER_H_
