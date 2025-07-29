#ifndef NV_TERM_H_
#define NV_TERM_H_

#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include "nv_unicode.h"

// Type of errors
typedef enum TermErrType {
    TermErrType_None = 0, // No error occurred
    TermErrType_Errno, // An internal error set by a C function
    TermErrType_CustomMsg // An error with a custom message in `data.customMsg`
} TermErrType;

// The error of a function call
typedef struct TermErr {
    TermErrType type; // The type of the error
    union {
        char *customMsg;
    } data;
} TermErr;

// TODO: improve key management

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
    TermKey_ArrowDown
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

/*********************************** Errors ***********************************/

// Get the current error of the library.
TermErr *termErr(void);
// Print the current error to stderr.
void termLogError(const char *msg);

/*********************************** Input ************************************/

// Get raw input characters, returns a negative value on error.
UcdCP termGetInput(void);
// Get key press, returns a negative value on error.
int32_t termGetKey(void);
// Read from the terminal.
int64_t termRead(UcdCh8 *buf, size_t bufSize);

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
