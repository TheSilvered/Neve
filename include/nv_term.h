#ifndef NV_TERM_H_
#define NV_TERM_H_

#include <stdbool.h>
#include <string.h>
#include <stdint.h>

// Type of errors
typedef enum TermErrType {
    TermErrType_none = 0, // No error occurred
    TermErrType_errno, // An internal error set by a C function
    TermErrType_customMsg // An error with a custom message in data.customMsg
} TermErrType;

// The current error of the library
typedef struct TermErr {
    TermErrType type; // The type of the error
    union {
        char *customMsg;
    } data;
} TermErr;

// Key pressed
typedef int TermKey;

// Check if a key is valid
#define termKeyOk(key) ((key) >= 0)
// Check if a key is an error
#define termKeyErr(key) ((key) < 0)

// Initialize library
bool termInit(void);
// Enable raw mode
// `getKeyTimeoutDSec` sets the timeout for termGetKey in tenths of a second,
// set to 0 disables timeout.
bool termEnableRawMode(uint8_t getKeyTimeoutDSec);
// Deinitialize library, restoring the terminal
void termQuit(void);

// Get the current error of the library
TermErr *termErr(void);
// Print the current error to stderr
void termLogError(const char *msg);

// Get the pressed key
TermKey termGetKey(void);

// The beginning of an escape sequence
#define TERM_ESC "\x1b["

// Write to the terminal
bool termWrite(const void *buf, size_t size);

#endif // !NV_TERM_H_
