#ifndef NV_UNICODE_H_
#define NV_UNICODE_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define ucdHighSurrogateFirst 0xD800
#define ucdHighSurrogateLast  0xDBFF

#define ucdLowSurrogateFirst 0xDC00
#define ucdLowSurrogateLast  0xDFFF

#define UcdCPMax 0x10FFFF

// UTF8 encoded character, a single character may not be a full codepoint.
typedef uint8_t  UcdCh8;
// UTF16 encoded character, a single character may not be a full codepoint.
typedef uint16_t UcdCh16;
// UTF32 encoded character (signed integer).
typedef int32_t UcdCh32;
// Unicode Codepoint (signed integer).
typedef UcdCh32 UcdCP;

// Return if a codepoint is valid.
// A valid codepoint is not a high or low surrogate and is not above U+10FFFF.
bool ucdIsCPValid(UcdCP cp);

// Get the width of a character (based on the East_Asian_Width property).
// If `currWidth` is `0` the width of `\t` will always be `tabStop`.
uint8_t ucdCPWidth(UcdCP cp, uint8_t tabStop, size_t currWidth);

// Translate a UTF16-encoded string to UTF8 adding a NUL character at the end.
// Encoding errors in `str` are ignored.
// Returns the length of the UTF8 string written to `buf`.
// If `buf` is NULL returns the size needed for `buf`.
size_t ucdCh16StrToCh8Str(
    const UcdCh16 *str, size_t strLen,
    UcdCh8 *buf, size_t bufLen
);

// Translate a UTF8-encoded string to UTF16 adding a NUL character at the end.
// Encoding errors in `str` are ignored.
// Returns the length of the UTF16 string written to `buf`.
// If `buf` is NULL returns the size needed for `buf`.
size_t ucdCh8StrToCh16Str(
    const UcdCh8 *str, size_t strLen,
    UcdCh16 *buf, size_t bufLen
);

// Get the total number of bytes of a UTF-8 codepoint given the first byte.
// Returns 0 if the byte is not the start of a UTF-8 sequence.
size_t ucdCh8RunLen(UcdCh8 firstCh);
// Get the total number of bytes needed to encode a codepoint in UTF-8.
// Returns 0 if the codepoint is not valid.
size_t ucdCh8CPLen(UcdCP cp);
// Decode a codepoint encoded in UTF-8.
UcdCP ucdCh8ToCP(const UcdCh8 *bytes);
// Encode a codepoint in UTF-8. Return the codepoint length.
size_t ucdCh8FromCP(UcdCP cp, UcdCh8 *outBuf);

// Get the total number of `UcdCh16` of a UTF-16 codepoint given the first one.
// Returns 0 if the character is not the start of a UTF-16 sequence.
size_t ucdCh16RunLen(UcdCh16 firstCh);
// Get the total number of `UcdCh16` needed to encode `cp` in UTF-16
// Returns 0 if the codepoint is not valid.
size_t ucdCh16CPLen(UcdCP cp);
// Decode a codepoint encoded in UTF-16.
UcdCP ucdCh16ToCP(const UcdCh16 *bytes);
// Encode a codepoint in UTF-16. Return the codepoint length.
size_t ucdCh16FromCP(UcdCP cp, UcdCh16 *outBuf);

#endif // !NV_UNICODE_H_
