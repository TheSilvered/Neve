#ifndef STERM_H_
#define STERM_H_

#include <stdbool.h>

// Type of errors
typedef enum TermErrType {
    TermErrType_none = 0, // No error occurred
    TermErrType_errno // An internal error set by a C function
} TermErrType;

// The current error of the library
typedef struct TermErr {
    TermErrType type; // The type of the error
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
bool termEnableRawMode(void);
// Deinitialize library, restoring the terminal
void termQuit(void);

// Get the current error of the library
TermErr *termErr(void);
// Print the current error to stderr
void termLogError(const char *msg);

// Get the pressed key
TermKey termGetKey(void);

#endif // !STERM_H_
