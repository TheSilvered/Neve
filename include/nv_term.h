#ifndef NV_TERM_H_
#define NV_TERM_H_

#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include "unicode/nv_utf.h"

// Special keys
typedef enum TermKey {
    TermKey_None = 0,
    TermKey_CtrlA,
    TermKey_CtrlB,
    TermKey_CtrlC,
    TermKey_CtrlD,
    TermKey_CtrlE,
    TermKey_CtrlF,
    TermKey_CtrlG,
    TermKey_CtrlH,
    TermKey_CtrlI,
    TermKey_CtrlJ,
    TermKey_CtrlK,
    TermKey_CtrlL,
    TermKey_CtrlM,
    TermKey_CtrlN,
    TermKey_CtrlO,
    TermKey_CtrlP,
    TermKey_CtrlQ,
    TermKey_CtrlR,
    TermKey_CtrlS,
    TermKey_CtrlT,
    TermKey_CtrlU,
    TermKey_CtrlV,
    TermKey_CtrlW,
    TermKey_CtrlX,
    TermKey_CtrlY,
    TermKey_CtrlZ,
    TermKey_Tab = TermKey_CtrlI,
    TermKey_Enter = TermKey_CtrlM,
    TermKey_Escape = 0x1b,
    TermKey_Backspace = 0x7f,
    TermKey_ArrowLeft = 0x110000,
    TermKey_ArrowRight,
    TermKey_ArrowUp,
    TermKey_ArrowDown,
    TermKey_F1,
    TermKey_F2,
    TermKey_F3,
    TermKey_F4,
    TermKey_F5,
    TermKey_F6,
    TermKey_F7,
    TermKey_F8,
    TermKey_F9,
    TermKey_F10,
    TermKey_F11,
    TermKey_F12,
    TermKey_Home,
    TermKey_End,
    TermKey_Delete
} TermKey;

/******************** Initialization and deinitialization *********************/

// Initialize the terminal.
bool termInit(void);
// Enable raw mode.
// `getInputTimeoutDSec` sets the timeout for `termGetInput` in tenths of a
// second, set to 0 disables timeout (can cause issues with `termGetKey`).
bool termEnableRawMode(uint8_t getInputTimeoutDSec);
// Deinitialize the terminal, restoring its previous state.
void termQuit(void);
// Check if the terminal is initialized.
bool termIsInit(void);

/*********************************** Input ************************************/

// Get raw input characters, returns a negative value on error.
UcdCP termGetInput(void);
// Get key press, returns a negative value on error.
int32_t termGetKey(void);
// Read from the terminal.
int64_t termRead(Utf8Ch *buf, size_t bufSize);

/*********************************** Output ***********************************/

// Write to the terminal.
bool termWrite(const void *buf, size_t size);

/************************************ Info ************************************/

// Get the size of the terminal.
bool termSize(uint16_t *outRows, uint16_t *outCols);
// Get the cursor position, (0, 0) is the top left corner.
bool termCursorGetPos(uint16_t *outX, uint16_t *outY);
// Set the cursor position, (0, 0) is the top left corner.
bool termCursorSetPos(uint16_t x, uint16_t y);

#endif // !NV_TERM_H_
