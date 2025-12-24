#ifndef NV_UTF_H_
#define NV_UTF_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "nv_ucd.h"

// UTF8 encoded character, a single character may not be a full codepoint.
typedef uint8_t  Utf8Ch;
// UTF16 encoded character, a single character may not be a full codepoint.
typedef uint16_t Utf16Ch;
// UTF32 encoded character (signed integer).
typedef int32_t Utf32Ch;

// Translate a UTF16-encoded string to UTF8 adding a NUL character at the end.
// Encoding errors in `str` are ignored.
// Returns the length of the UTF8 string written to `buf`.
// If `buf` is NULL returns the size needed for `buf`.
size_t utf16ToUtf8(
    const Utf16Ch *str, size_t strLen,
    Utf8Ch *buf, size_t bufLen
);

// Translate a UTF8-encoded string to UTF16 adding a NUL character at the end.
// Encoding errors in `str` are ignored.
// Returns the length of the UTF16 string written to `buf`.
// If `buf` is NULL returns the size needed for `buf`.
size_t utf8ToUtf16(
    const Utf8Ch *str, size_t strLen,
    Utf16Ch *buf, size_t bufLen
);

// Get the total number of bytes of a UTF-8 codepoint given the first byte.
// Returns 0 if the byte is not the start of a UTF-8 sequence.
uint8_t utf8ChRunLen(Utf8Ch firstCh);
// Get the total number of bytes needed to encode a codepoint in UTF-8.
// Returns 0 if the codepoint is not valid.
uint8_t utf8CPLen(UcdCP cp);
// Decode a codepoint encoded in UTF-8.
UcdCP utf8ChToCP(const Utf8Ch *bytes);
// Encode a codepoint in UTF-8. Return the codepoint length.
uint8_t utf8FromCP(UcdCP cp, Utf8Ch *outBuf);
// Check if the byte is a valid UTF-8 sequence start.
bool utf8ChIsStart(Utf8Ch ch);
// Check if a string is correct UTF-8.
bool utf8Check(const Utf8Ch *str, size_t strLen);

// Get the total number of `UcdCh16` of a UTF-16 codepoint given the first one.
// Returns 0 if the character is not the start of a UTF-16 sequence.
uint8_t utf16ChRunLen(Utf16Ch firstCh);
// Get the total number of `UcdCh16` needed to encode `cp` in UTF-16
// Returns 0 if the codepoint is not valid.
uint8_t utf16CPLen(UcdCP cp);
// Decode a codepoint encoded in UTF-16.
UcdCP utf16ChToCP(const Utf16Ch *bytes);
// Encode a codepoint in UTF-16. Return the codepoint length.
uint8_t utf16FromCP(UcdCP cp, Utf16Ch *outBuf);

#endif // !NV_UTF_H_
