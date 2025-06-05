#ifndef NV_STR_H_
#define NV_STR_H_

#include <stddef.h>
#include "nv_unicode.h"

// A heap-allocated string
// Any function that expects a StrView * also accepts a Str *
typedef struct Str {
    UcdCh8 *buf;
    size_t len;
    size_t cap;
} Str;

// A string view (does not own the memory)
typedef struct StrView {
    const UcdCh8 *buf;
    size_t len;
} StrView;

// A string that owns a block of memory but does not allocate/deallocate it
typedef struct StrBuf {
    UcdCh8 *buf;
    size_t len;
    size_t bufSize;
} StrBuf;

// Allocate a new empty string
// Set `reserve` to avoid reallocations
Str *strNew(size_t reserve);
// Allocate a new string from a C string
Str *strNewFromC(const char *cStr);
// Initialize a new empty string
// Set `reserve` to avoid reallcations, a reserve=0 will not allocate memory
bool strInit(Str *str, size_t reserve);
// Initialize a new string from a C string
bool strInitFromC(Str *str, const char *cStr);
// Free a heap allocated string
void strFree(Str *str);
// Destroy the contents of a string
void strDestroy(Str *str);
// Reserve some bytes to the end of the string to avoid excessive reallocations
bool strReserve(Str *str, size_t reserve);
// Append a C string to a string
bool strAppendC(Str *str, const char *cStr);
// Append a string view to a string
bool strAppend(Str *str, StrView *sv);
// Clear the contents of a string and keep a capacity of `reserve`
bool strClear(Str *str, size_t reserve);
// Get the contents of a string as a NUL terminated string
char *strAsC(Str *str);

// Initialize a new string view from a C string
void strViewInitFromC(StrView *sv, const char *cStr);
// Make a new StrView from a C string
StrView strViewMakeFromC(const char *cStr);

// Initialize a new string buffer
void strBufInit(StrBuf *sb, UcdCh8 *buf, size_t bufSize);
// Make a new string buffer
StrBuf strBufMake(UcdCh8 *buf, size_t bufSize);
// Append a C string to a string buffer
bool strBufAppendC(StrBuf *sb, const char *cStr);
// Append a string view to a string buffer
bool strBufAppend(StrBuf *sb, StrView *sv);
// Clear the contents of a string buffer
void strBufClear(StrBuf *sb);

#endif // !NV_STR_H_
