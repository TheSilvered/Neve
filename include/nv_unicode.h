#ifndef NV_UNICODE_H_
#define NV_UNICODE_H_

#include <stdint.h>
#include <stddef.h>

// UTF8 encoded character, a single character may not be a full codepoint
typedef uint8_t  UcdCh8;
// UTF16 encoded character, a single character may not be a full codepoint
typedef uint16_t UcdCh16;
// UTF32 encoded character
typedef uint32_t UcdCh32;

// Translate a UTF16-encoded string to UTF8 adding a NUL character at the end.
// Encoding errors in `str` are ignored.
// Returns the length of the UTF8 string written to `buf`.
size_t ucdUTF16ToUTF8(
    UcdCh16 *str, size_t strLen,
    UcdCh8  *buf, size_t bufLen
);

#endif // !NV_UNICODE_H_
