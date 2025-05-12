#include "nv_unicode.h"

// UTF-8 encoding table
//
// | First CP | Last CP  |  Byte 1  |  Byte 2  |  Byte 3  |  Byte 4  |
// | -------- | -------- | -------- | -------- | -------- | -------- |
// |   U+0000 |   U+007F | 0xxxxxxx |          |          |          |
// |   U+0080 |   U+07FF | 110xxxxx | 10xxxxxx |          |          |
// |   U+0800 |   U+FFFF | 1110xxxx | 10xxxxxx | 10xxxxxx |          |
// |  U+10000 | U+10FFFF | 11110xxx | 10xxxxxx | 10xxxxxx | 10xxxxxx |
//

// Mask for the data of the first byte of a 2-byte character sequence
#define UTF8_ByteMask2 0b00011111
// Mask for the data of the first byte of a 3-byte character sequence
#define UTF8_ByteMask3 0b00001111
// Mask for the data of the first byte of a 4-byte character sequence
#define UTF8_ByteMask4 0b00000111
// Mask for the data of following bytes in a character sequence
#define UTF8_ByteMaskX 0b00111111

size_t ucdUTF16ToUTF8(
    UcdCh16 *str, size_t strLen,
    UcdCh8  *buf, size_t bufLen
) {
    size_t bufIdx = 0;
    for (size_t strIdx = 0; strIdx < strLen;) {
        UcdCh16 wch = str[strIdx++];
        UcdCh32 ch = 0;
        if (wch < 0xd800 || wch > 0xdfff) {
            ch = wch;
        } else {
            if (strIdx == strLen) {
                break;
            }
            ch = ((wch & 0x3ff) << 10)
                      + (str[strIdx++] & 0x3ff)
                      + 0x10000;
        }

        // ignore invalid characters
        if (ch > 0x10ffff) {
            continue;
        }

        // all lenth checks use '>' instead of '>=' because an extra NUL
        // character is added at the end

        if (buf == NULL) {
            bufIdx += ucdUTF8ChLen(ch);
        } else if (ch <= 0x7f && bufLen - bufIdx > 1) {
            buf[bufIdx++] = (UcdCh8)ch;
        } else if (ch <= 0x7ff && bufLen - bufIdx > 2) {
            buf[bufIdx++] = 0b11000000 | (UcdCh8)(ch >> 6);
            buf[bufIdx++] = 0b10000000 | (UcdCh8)(ch & 0x3f);
        } else if (ch <= 0xffff && bufLen - bufIdx > 3) {
            buf[bufIdx++] = 0b11100000 | (UcdCh8)(ch >> 12);
            buf[bufIdx++] = 0b10000000 | (UcdCh8)(ch >> 6 & 0x3f);
            buf[bufIdx++] = 0b10000000 | (UcdCh8)(ch & 0x3f);
        } else if (ch <= 0x10ffff && bufLen - bufIdx > 4) {
            buf[bufIdx++] = 0b11110000 | (UcdCh8)(ch >> 18);
            buf[bufIdx++] = 0b10000000 | (UcdCh8)(ch >> 12 & 0x3f);
            buf[bufIdx++] = 0b10000000 | (UcdCh8)(ch >> 6 & 0x3f);
            buf[bufIdx++] = 0b10000000 | (UcdCh8)(ch & 0x3f);
        } else
            break;
    }
    if (buf == NULL)
        return bufIdx + 1;

    buf[bufIdx] = '\0';
    return bufIdx;
}

size_t ucdUTF8ByteLen(UcdCh8 byte0) {
    return (byte0 < 0x80)
         + 2*((byte0 & ~UTF8_ByteMask2) == 0b11000000)
         + 3*((byte0 & ~UTF8_ByteMask3) == 0b11100000)
         + 4*((byte0 & ~UTF8_ByteMask4) == 0b11110000);
}

size_t ucdUTF8ChLen(UcdCh32 ch) {
    return (ch < 0x80)
         + 2*(ch >=    0x80 && ch <    0x800)
         + 3*(ch >=   0x800 && ch <  0x10000)
         + 4*(ch >= 0x10000 && ch < 0x10ffff);
}

UcdCh32 ucdCh8ToCh32(UcdCh8 *bytes) {
    switch (ucdUTF8ByteLen(bytes[0])) {
    case 1:
        return (UcdCh32)bytes[0];
    case 2:
        return ((UcdCh32)(bytes[0] & UTF8_ByteMask2) << 6)
             |  (UcdCh32)(bytes[1] & UTF8_ByteMaskX);
    case 3:
        return ((UcdCh32)(bytes[0] & UTF8_ByteMask3) << 12)
             | ((UcdCh32)(bytes[1] & UTF8_ByteMaskX) << 6)
             |  (UcdCh32)(bytes[2] & UTF8_ByteMaskX);
    case 4:
        return ((UcdCh32)(bytes[0] & UTF8_ByteMask3) << 18)
             | ((UcdCh32)(bytes[1] & UTF8_ByteMaskX) << 12)
             | ((UcdCh32)(bytes[2] & UTF8_ByteMaskX) << 6)
             |  (UcdCh32)(bytes[3] & UTF8_ByteMaskX);
    default:
        return 0;
    }
}
