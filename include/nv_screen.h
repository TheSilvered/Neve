#ifndef NV_RENDER_H_
#define NV_RENDER_H_

#include "nv_string.h"
#include "nv_utils.h"

// 16-colors color mode
#define screenColModeT16 0
// 256-colors color mode
#define screenColModeT256 1
// 24-bit RGB colors color mode
#define screenColModeRGB 2

#define screenColRGB(r_, g_, b_) { .r = (r_), .g = (g_), .b = (b_) }
#define screenColT16(col) { .r = (col) }
#define screenColT256(col) { .r = (col) }

// The color of one cell
// The IDs for the TERM16 color mode are the following:
// 0 - default
// 1 - black
// 2 - red
// 3 - green
// 4 - yellow
// 5 - blue
// 6 - magenta
// 7 - cyan
// 8 - white
// 61..68 are the bright variants.
// This is to make `(ScreenStyle){ 0 }` the default style.
// Set `fg` and `bg` with the `screenCol*` macros.
typedef struct {
    struct {
        uint8_t r, g, b;
    } fg, bg;
    unsigned int bold : 1;
    unsigned int underline : 1;
    unsigned int italic : 1;
    unsigned int strike: 1;
    unsigned int reverse : 1;
    unsigned int fgColorMode : 2;
    unsigned int bgColorMode : 2;
} ScreenStyle;

// The painting surface.
typedef struct {
    uint16_t w, h;
    Str buf;
    Str *editRows;
    Str *displayRows;
    ScreenStyle *editStyles;
    ScreenStyle *displayStyles;
    bool resized;
} Screen;

typedef struct {
    uint16_t x, y, w, h;
} ScreenRect;

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
NV_UNIX_FMT(4, 5) void screenWriteFmt(
    Screen *screen,
    uint16_t x, uint16_t y,
    NV_WIN_FMT const char *fmt, ...
);

// Change the style for a rectangle of the screen
void screenSetStyle(Screen *screen, ScreenStyle st, ScreenRect rect);

// Clear a line of the screen.
// If `line == -1` all lines are cleared.
void screenClear(Screen *screen, int32_t line);

// Display any draw calls to the screen.
bool screenRefresh(Screen *screen);

#endif // !NV_RENDER_H_

