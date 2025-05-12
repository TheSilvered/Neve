#ifndef NV_UNICODE_H_
#define NV_UNICODE_H_

#include <stdint.h>
#include <stddef.h>

#define UCD_HIGH_SURROGATE_FIRST 0xD800
#define UCD_HIGH_SURROGATE_LAST  0xDBFF

#define UCD_LOW_SURROGATE_FIRST 0xDC00
#define UCD_LOW_SURROGATE_LAST  0xDFFF

// UTF8 encoded character, a single character may not be a full codepoint
typedef uint8_t  UcdCh8;
// UTF16 encoded character, a single character may not be a full codepoint
typedef uint16_t UcdCh16;
// UTF32 encoded character
typedef uint32_t UcdCh32;

// Translate a UTF16-encoded string to UTF8 adding a NUL character at the end.
// Encoding errors in `str` are ignored.
// Returns the length of the UTF8 string written to `buf`.
// If `buf` is NULL returns the size needed for `buf`
size_t ucdUTF16ToUTF8(
    UcdCh16 *str, size_t strLen,
    UcdCh8  *buf, size_t bufLen
);

// Get the total number of bytes of a UTF8 character given the first byte.
// Returns 0 if the byte is not the start of a UTF8 sequence.
size_t ucdUTF8ByteLen(UcdCh8 byte0);
// Get the total number of bytes needed to encode `ch` in UTF-8
// Returns 0 if the character is not valid.
size_t ucdUTF8ChLen(UcdCh32 ch);
// Decode a character encoded in UTF-8.
UcdCh32 ucdCh8ToCh32(UcdCh8 *bytes);

// Decode a character encoded in UTF-16.
UcdCh32 ucdCh16ToCh32(UcdCh16 *bytes);

#endif // !NV_UNICODE_H_
