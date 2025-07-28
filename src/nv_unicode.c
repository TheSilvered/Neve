#include <assert.h>

#include "nv_unicode.h"
#include "nv_udb.h"

// UTF-8 encoding table
//
// | First CP | Last CP  |  Byte 1  |  Byte 2  |  Byte 3  |  Byte 4  |
// | -------- | -------- | -------- | -------- | -------- | -------- |
// |   U+0000 |   U+007F | 0xxxxxxx |          |          |          |
// |   U+0080 |   U+07FF | 110xxxxx | 10xxxxxx |          |          |
// |   U+0800 |   U+FFFF | 1110xxxx | 10xxxxxx | 10xxxxxx |          |
// |  U+10000 | U+10FFFF | 11110xxx | 10xxxxxx | 10xxxxxx | 10xxxxxx |

// Mask for the data of the first byte of a 2-byte character sequence
#define UTF8_ByteMask2 0x1f // 0b00011111
// Mask for the data of the first byte of a 3-byte character sequence
#define UTF8_ByteMask3 0x0f // 0b00001111
// Mask for the data of the first byte of a 4-byte character sequence
#define UTF8_ByteMask4 0x07 // 0b00000111
// Mask for the data of following bytes in a character sequence
#define UTF8_ByteMaskX 0x3f // 0b00111111

#define DECODING_ERROR_CH 0xfffd

bool ucdIsCPValid(UcdCP cp) {
    return cp <= 0x10ffff
        && cp >= 0
        && (cp < ucdHighSurrogateFirst || cp > ucdLowSurrogateLast);
}

uint8_t ucdCPWidth(UcdCP cp) {
    // Short path for ASCII printable characters
    if (cp >= ' ' && cp <= '~') {
        return 1;
    }

    {
        UdbCPInfo info = udbGetCPInfo(cp);
        switch (info.width) {
        case UdbWidth_Fullwidth:
        case UdbWidth_Wide:
            return 2;
        case UdbWidth_Ambiguous:
        case UdbWidth_Neutral:
        case UdbWidth_Narrow:
        case UdbWidth_Halfwidth:
            return 1;
        default:
            assert(false);
            return 0;
        }
    }
}

size_t ucdCh16StrToCh8Str(
    const UcdCh16 *str, size_t strLen,
    UcdCh8 *buf, size_t bufLen
) {
    size_t bufIdx = 0;
    for (size_t strIdx = 0; strIdx < strLen;) {
        UcdCh16 wch = str[strIdx++];
        UcdCP ch = 0;
        if (wch < ucdHighSurrogateFirst || wch > ucdHighSurrogateLast) {
            ch = wch;
        } else if (
            strIdx == strLen
            || str[strIdx] < ucdLowSurrogateFirst
            || str[strIdx] > ucdLowSurrogateLast
        ) {
            ch = DECODING_ERROR_CH;
        } else {
            ch = ((wch & 0x3ff) << 10) + (str[strIdx++] & 0x3ff) + 0x10000;
        }

        // replace invalid characters with U+FFFD
        if (!ucdIsCPValid(ch)) {
            ch = DECODING_ERROR_CH;
        }

        // all lenth checks use '>' instead of '>=' because an extra NUL
        // character is added at the end

        if (buf == NULL) {
            bufIdx += ucdCh8CPLen(ch);
        } else if (ch <= 0x7f && bufLen - bufIdx > 1) {
            buf[bufIdx++] = (UcdCh8)ch;
        } else if (ch <= 0x7ff && bufLen - bufIdx > 2) {
            buf[bufIdx++] = 0xc0 | (UcdCh8)(ch >> 6);
            buf[bufIdx++] = 0x80 | (UcdCh8)(ch & 0x3f);
        } else if (ch <= 0xffff && bufLen - bufIdx > 3) {
            buf[bufIdx++] = 0xe0 | (UcdCh8)(ch >> 12);
            buf[bufIdx++] = 0x80 | (UcdCh8)(ch >> 6 & 0x3f);
            buf[bufIdx++] = 0x80 | (UcdCh8)(ch & 0x3f);
        } else if (ch <= 0x10ffff && bufLen - bufIdx > 4) {
            buf[bufIdx++] = 0xf0 | (UcdCh8)(ch >> 18);
            buf[bufIdx++] = 0x80 | (UcdCh8)(ch >> 12 & 0x3f);
            buf[bufIdx++] = 0x80 | (UcdCh8)(ch >> 6 & 0x3f);
            buf[bufIdx++] = 0x80 | (UcdCh8)(ch & 0x3f);
        } else
            break;
    }
    if (buf == NULL)
        return bufIdx + 1;

    buf[bufIdx] = '\0';
    return bufIdx;
}

size_t ucdCh8StrToCh16Str(
    const UcdCh8 *str, size_t strLen,
    UcdCh16 *buf, size_t bufLen
) {
    size_t bufIdx = 0;
    for (size_t strIdx = 0; strIdx < strLen;) {
        UcdCh8 ch8 = str[strIdx++];
        UcdCP ch = 0;

        switch (ucdCh8RunLen(ch8)) {
        case 0: // invalid byte
            ch = DECODING_ERROR_CH;
            break;
        case 1:
            ch = ch8;
            break;
        case 2:
            if (
                strIdx == strLen
                || (str[strIdx] & (~UTF8_ByteMaskX)) != 0x80
            ) {
                ch = DECODING_ERROR_CH;
                break;
            }
            ch = ((ch8 & UTF8_ByteMask2) << 6)
               | (str[strIdx++] & UTF8_ByteMaskX);
            // ch cannot be < 0x80 (see ucdCh8RunLen)
            break;
        case 3:
            if (
                strIdx >= strLen - 1
                || (str[strIdx] & (~UTF8_ByteMaskX)) != 0x80
                || (str[strIdx + 1] & (~UTF8_ByteMaskX)) != 0x80
            ) {
                ch = DECODING_ERROR_CH;
                break;
            }

            ch = ((ch8 & UTF8_ByteMask3) << 12);
            ch |= ((str[strIdx++] & UTF8_ByteMaskX) << 6);
            ch |= (str[strIdx++] & UTF8_ByteMaskX);
            if (ch < 0x800) {
                ch = DECODING_ERROR_CH;
            }
            break;
        case 4:
            if (
                strIdx >= strLen - 2
                || (str[strIdx] & (~UTF8_ByteMaskX)) != 0x80
                || (str[strIdx + 1] & (~UTF8_ByteMaskX)) != 0x80
                || (str[strIdx + 2] & (~UTF8_ByteMaskX)) != 0x80
            ) {
                ch = DECODING_ERROR_CH;
                break;
            }

            ch = ((ch8 & UTF8_ByteMask4) << 18);
            ch |= ((str[strIdx++] & UTF8_ByteMaskX) << 12);
            ch |= ((str[strIdx++] & UTF8_ByteMaskX) << 6);
            ch |= (str[strIdx++] & UTF8_ByteMaskX);
            if (ch < 0x10000) {
                ch = DECODING_ERROR_CH;
            }
            break;
        default:
            assert(false);
        }

        // replace invalid characters with U+FFFD
        if (!ucdIsCPValid(ch)) {
            ch = DECODING_ERROR_CH;
        }

        // all lenth checks use '>' instead of '>=' because an extra NUL
        // character is added at the end

        if (buf == NULL) {
            bufIdx += ucdCh16CPLen(ch);
        } else if (ch <= 0xffff && bufLen - bufIdx > 1) {
            buf[bufIdx++] = (UcdCh16)ch;
        } else if (ch <= 0x10ffff && bufLen - bufIdx > 2) {
            ch -= 0x10000;
            buf[bufIdx++] = ucdHighSurrogateFirst | (UcdCh16)(ch >> 10);
            buf[bufIdx++] = ucdLowSurrogateFirst | (UcdCh16)(ch & 0x3ff);
        } else
            break;
    }
    if (buf == NULL)
        return bufIdx + 1;

    buf[bufIdx] = 0;
    return bufIdx;
}

size_t ucdCh8RunLen(UcdCh8 byte0) {
    return (byte0 < 0x80)
         + 2*((byte0 & ~UTF8_ByteMask2) == 0xc0)*(byte0 >= 0xc2)
         + 3*((byte0 & ~UTF8_ByteMask3) == 0xe0)
         + 4*((byte0 & ~UTF8_ByteMask4) == 0xf0)*(byte0 <= 0xf4);
}

size_t ucdCh8CPLen(UcdCP ch) {
    return (ch >= 0 && ch < 0x80)
         + 2*(!!(ch >= 0x80 && ch < 0x800))
         + 3*(!!(ch >= 0x800 && ch < ucdHighSurrogateFirst))
         + 3*(!!(ch > ucdLowSurrogateLast && ch < 0x10000))
         + 4*(!!(ch >= 0x10000 && ch <= 0x10ffff));
}

UcdCP ucdCh8ToCP(const UcdCh8 *bytes) {
    switch (ucdCh8RunLen(bytes[0])) {
    case 1:
        return (UcdCP)bytes[0];
    case 2:
        return ((UcdCP)(bytes[0] & UTF8_ByteMask2) << 6)
             |  (UcdCP)(bytes[1] & UTF8_ByteMaskX);
    case 3:
        return ((UcdCP)(bytes[0] & UTF8_ByteMask3) << 12)
             | ((UcdCP)(bytes[1] & UTF8_ByteMaskX) << 6)
             |  (UcdCP)(bytes[2] & UTF8_ByteMaskX);
    case 4:
        return ((UcdCP)(bytes[0] & UTF8_ByteMask4) << 18)
             | ((UcdCP)(bytes[1] & UTF8_ByteMaskX) << 12)
             | ((UcdCP)(bytes[2] & UTF8_ByteMaskX) << 6)
             |  (UcdCP)(bytes[3] & UTF8_ByteMaskX);
    default:
        return DECODING_ERROR_CH;
    }
}

size_t ucdCh16RunLen(UcdCh16 firstCh) {
    return (
        1 + (!!(
            firstCh >= ucdHighSurrogateFirst
            && firstCh <= ucdHighSurrogateLast
        ))
    ) * (!!(
        firstCh < ucdLowSurrogateFirst || firstCh > ucdLowSurrogateLast
    ));
}

size_t ucdCh16CPLen(UcdCP cp) {
    return (1 + (cp > 0xffff)) * ucdIsCPValid(cp);
}

UcdCh32 ucdCh16ToCP(const UcdCh16 *bytes) {
    if (
        bytes[0] < ucdHighSurrogateFirst
        || bytes[0] > ucdHighSurrogateLast
    ) {
        return bytes[0];
    }

    if (
        bytes[0] >= ucdLowSurrogateFirst
        && bytes[0] <= ucdLowSurrogateLast
    ) {
        return DECODING_ERROR_CH;
    }

    return ((bytes[0] & 0x3ff) << 10) + (bytes[1] & 0x3ff) + 0x10000;
}
