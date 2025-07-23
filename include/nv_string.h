#ifndef NV_STRING_H_
#define NV_STRING_H_

#include <stddef.h>
#include "nv_unicode.h"

// A heap-allocated string.
// Any function that expects a `StrView *` also accepts a `Str *`.
// Use `strAsC` instead of reading the contents of `buf` directly.
typedef struct Str {
    UcdCh8 *buf;
    size_t len;
    size_t cap;
} Str;

// A string view (does not own the memory).
// `buf` is not guaranteed to end with a NUL character.
typedef struct StrView {
    const UcdCh8 *buf;
    size_t len;
} StrView;

// A string that owns a block of memory but does not allocate/deallocate it.
// Any function that expects a `StrView *` also accepts a `StrBuf *`.
typedef struct StrBuf {
    char *buf;
    size_t len;
    size_t bufSize;
} StrBuf;

// Allocate a new empty string.
// Set `reserve` to avoid reallocations.
Str *strNew(size_t reserve);
// Allocate a new string from a C string.
Str *strNewFromC(const char *cStr);
// Initialize a new empty string.
// Set `reserve` to avoid reallcations, a reserve=0 will not allocate memory.
void strInit(Str *str, size_t reserve);
// Initialize a new string from a C string.
void strInitFromC(Str *str, const char *cStr);
// Free a heap allocated string.
void strFree(Str *str);
// Destroy the contents of a string.
void strDestroy(Str *str);
// Reserve some bytes to the end of the string to avoid excessive reallocations.
void strReserve(Str *str, size_t reserve);
// Append a C string to a string.
void strAppendC(Str *str, const char *cStr);
// Append a string view to a string.
void strAppend(Str *str, const StrView *sv);
// Clear the contents of a string and keep a capacity of `reserve`.
void strClear(Str *str, size_t reserve);
// Get the contents of a string as a NUL terminated string.
char *strAsC(Str *str);

// Initialize a new string view from a C string.
void strViewInitFromC(StrView *sv, const char *cStr);
// Make a new StrView from a C string.
StrView strViewMakeFromC(const char *cStr);

// Initialize a new string buffer.
void strBufInit(StrBuf *sb, char *buf, size_t bufSize);
// Make a new string buffer.
StrBuf strBufMake(char *buf, size_t bufSize);
// Append a C string to a string buffer.
bool strBufAppendC(StrBuf *sb, const char *cStr);
// Append a string view to a string buffer.
bool strBufAppend(StrBuf *sb, const StrView *sv);
// Clear the contents of a string buffer.
void strBufClear(StrBuf *sb);

// TODO: add strViewNext tests

// Iterate through the codepoints of a `StrView`
// Usage:
// ```c
// for (
//     ptrdiff_t i = strViewNext(&sv, -1, &cp);
//     i != -1;
//     i = strViewNext(&sv, i, &cp)
// ) { ... }
// ```
ptrdiff_t strViewNext(StrView *sv, ptrdiff_t idx, UcdCP *outCP);

#ifdef _WIN32
// Windows only. Temporary wchar_t string from char.
// The string is valid until the next call of the funcion.
const wchar_t *tempWStr(const char *str);

// Windows only. Temporary char string from wchar_t.
// The string is valid until the next call of the funcion.
const char *tempStr(const wchar_t *str);
#endif // !_WIN32

#endif // !NV_STRING_H_
