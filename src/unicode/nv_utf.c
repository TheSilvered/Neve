#include <assert.h>

#include "unicode/nv_utf.h"
#include "unicode/nv_ucd.h"
#include "nv_utils.h"

// UTF-8 encoding table
//
// | First CP | Last CP  |  Byte 1  |  Byte 2  |  Byte 3  |  Byte 4  |
// | -------- | -------- | -------- | -------- | -------- | -------- |
// |   U+0000 |   U+007F | 0xxxxxxx |          |          |          |
// |   U+0080 |   U+07FF | 110xxxxx | 10xxxxxx |          |          |
// |   U+0800 |   U+FFFF | 1110xxxx | 10xxxxxx | 10xxxxxx |          |
// |  U+10000 | U+10FFFF | 11110xxx | 10xxxxxx | 10xxxxxx | 10xxxxxx |

// Mask for the data of the first byte of a 2-byte character sequence
#define _utf8ByteMask2 0x1f // 0b00011111
// Mask for the data of the first byte of a 3-byte character sequence
#define _utf8ByteMask3 0x0f // 0b00001111
// Mask for the data of the first byte of a 4-byte character sequence
#define _utf8ByteMask4 0x07 // 0b00000111
// Mask for the data of following bytes in a character sequence
#define _utf8ByteMaskX 0x3f // 0b00111111

#define _decodingErrorCh 0xfffd

typedef struct Utf8ChCheckResult {
    UcdCP cp;
    uint8_t advance;
    bool isValid;
} Utf8ChCheckResult;

static Utf8ChCheckResult _ucdCh8CheckSingle(
    const Utf8Ch *str,
    size_t len,
    size_t idx
) {
    Utf8Ch ch8 = str[idx++];
    UcdCP ch = 0;
    uint8_t advance = 1;
    UcdCP cp = _decodingErrorCh;
    bool isValid = false;

    switch (utf8ChRunLen(ch8)) {
    case 0:
        break;
    case 1:
        cp = ch8;
        isValid = true;
        break;
    case 2:
        if (idx == len || (str[idx] & (~_utf8ByteMaskX)) != 0x80) {
            break;
        }
        advance = 2;

        ch = ((ch8        & _utf8ByteMask2) << 6);
        ch |= (str[idx++] & _utf8ByteMaskX);
        if (ch >= 0x80) {
            cp = ch;
            isValid = true;
        }
        break;
    case 3:
        if (
            idx >= len - 1
            || (str[idx] & (~_utf8ByteMaskX)) != 0x80
            || (str[idx + 1] & (~_utf8ByteMaskX)) != 0x80
        ) {
            break;
        }
        advance = 3;

        ch  = ((ch8        & _utf8ByteMask3) << 12);
        ch |= ((str[idx++] & _utf8ByteMaskX) << 6);
        ch |=  (str[idx++] & _utf8ByteMaskX);
        if (ch >= 0x800
            && (ch < ucdHighSurrogateFirst || ch > ucdLowSurrogateLast)
        ) {
            cp = ch;
            isValid = true;
        }
        break;
    case 4:
        if (
            idx >= len - 2
            || (str[idx] & (~_utf8ByteMaskX)) != 0x80
            || (str[idx + 1] & (~_utf8ByteMaskX)) != 0x80
            || (str[idx + 2] & (~_utf8ByteMaskX)) != 0x80
        ) {
            break;
        }
        advance = 4;

        ch  = ((ch8        & _utf8ByteMask4) << 18);
        ch |= ((str[idx++] & _utf8ByteMaskX) << 12);
        ch |= ((str[idx++] & _utf8ByteMaskX) << 6);
        ch |=  (str[idx++] & _utf8ByteMaskX);
        if (ch >= 0x10000 && ch <= ucdCPMax) {
            cp = ch;
            isValid = true;
        }
        break;
    default:
        nvUnreachable;
        break;
    }

    return (Utf8ChCheckResult) {
        .advance = advance,
        .cp = cp,
        .isValid = isValid
    };
}

size_t utf16ToUtf8(
    const Utf16Ch *str, size_t strLen,
    Utf8Ch *buf, size_t bufLen
) {
    size_t bufIdx = 0;
    for (size_t strIdx = 0; strIdx < strLen;) {
        Utf16Ch wch = str[strIdx++];
        UcdCP ch = 0;
        if (wch < ucdHighSurrogateFirst || wch > ucdHighSurrogateLast) {
            ch = wch;
        } else if (
            strIdx == strLen
            || str[strIdx] < ucdLowSurrogateFirst
            || str[strIdx] > ucdLowSurrogateLast
        ) {
            ch = _decodingErrorCh;
        } else {
            ch = ((wch & 0x3ff) << 10) + (str[strIdx++] & 0x3ff) + 0x10000;
        }

        // replace invalid characters with U+FFFD
        if (!ucdIsCPValid(ch)) {
            ch = _decodingErrorCh;
        }

        // all lenth checks use '>' instead of '>=' because an extra NUL
        // character is added at the end

        if (buf == NULL) {
            bufIdx += utf8CPLen(ch);
        } else if (ch <= 0x7f && bufLen - bufIdx > 1) {
            buf[bufIdx++] = (Utf8Ch)ch;
        } else if (ch <= 0x7ff && bufLen - bufIdx > 2) {
            buf[bufIdx++] = 0xc0 | (Utf8Ch)(ch >> 6);
            buf[bufIdx++] = 0x80 | (Utf8Ch)(ch & 0x3f);
        } else if (ch <= 0xffff && bufLen - bufIdx > 3) {
            buf[bufIdx++] = 0xe0 | (Utf8Ch)(ch >> 12);
            buf[bufIdx++] = 0x80 | (Utf8Ch)(ch >> 6 & 0x3f);
            buf[bufIdx++] = 0x80 | (Utf8Ch)(ch & 0x3f);
        } else if (ch <= ucdCPMax && bufLen - bufIdx > 4) {
            buf[bufIdx++] = 0xf0 | (Utf8Ch)(ch >> 18);
            buf[bufIdx++] = 0x80 | (Utf8Ch)(ch >> 12 & 0x3f);
            buf[bufIdx++] = 0x80 | (Utf8Ch)(ch >> 6 & 0x3f);
            buf[bufIdx++] = 0x80 | (Utf8Ch)(ch & 0x3f);
        } else {
            break;
        }
    }

    if (buf == NULL) {
        return bufIdx + 1;
    }

    buf[bufIdx] = '\0';
    return bufIdx;
}

size_t utf8ToUtf16(
    const Utf8Ch *str, size_t strLen,
    Utf16Ch *buf, size_t bufLen
) {
    size_t bufIdx = 0;
    for (size_t strIdx = 0; strIdx < strLen;) {
        Utf8ChCheckResult res = _ucdCh8CheckSingle(str, strLen, strIdx);
        strIdx += res.advance;
        UcdCP ch = res.cp;

        // all lenth checks use '>' instead of '>=' because an extra NUL
        // character is added at the end

        if (buf == NULL) {
            bufIdx += utf16CPLen(ch);
        } else if (ch <= 0xffff && bufLen - bufIdx > 1) {
            buf[bufIdx++] = (Utf16Ch)ch;
        } else if (ch <= ucdCPMax && bufLen - bufIdx > 2) {
            ch -= 0x10000;
            buf[bufIdx++] = ucdHighSurrogateFirst | (Utf16Ch)(ch >> 10);
            buf[bufIdx++] = ucdLowSurrogateFirst | (Utf16Ch)(ch & 0x3ff);
        } else {
            break;
        }
    }

    if (buf == NULL) {
        return bufIdx + 1;
    }

    buf[bufIdx] = 0;
    return bufIdx;
}

uint8_t utf8ChRunLen(Utf8Ch byte0) {
    return (byte0 < 0x80)
         + 2*((byte0 & ~_utf8ByteMask2) == 0xc0)*(byte0 >= 0xc2)
         + 3*((byte0 & ~_utf8ByteMask3) == 0xe0)
         + 4*((byte0 & ~_utf8ByteMask4) == 0xf0)*(byte0 <= 0xf4);
}

uint8_t utf8CPLen(UcdCP ch) {
    return (ch >= 0 && ch < 0x80)
         + 2*(!!(ch >= 0x80 && ch < 0x800))
         + 3*(!!(ch >= 0x800 && ch < ucdHighSurrogateFirst))
         + 3*(!!(ch > ucdLowSurrogateLast && ch < 0x10000))
         + 4*(!!(ch >= 0x10000 && ch <= ucdCPMax));
}

UcdCP utf8ChToCP(const Utf8Ch *bytes) {
    switch (utf8ChRunLen(bytes[0])) {
    case 1:
        return (UcdCP)bytes[0];
    case 2:
        return ((UcdCP)(bytes[0] & _utf8ByteMask2) << 6)
             |  (UcdCP)(bytes[1] & _utf8ByteMaskX);
    case 3:
        return ((UcdCP)(bytes[0] & _utf8ByteMask3) << 12)
             | ((UcdCP)(bytes[1] & _utf8ByteMaskX) << 6)
             |  (UcdCP)(bytes[2] & _utf8ByteMaskX);
    case 4:
        return ((UcdCP)(bytes[0] & _utf8ByteMask4) << 18)
             | ((UcdCP)(bytes[1] & _utf8ByteMaskX) << 12)
             | ((UcdCP)(bytes[2] & _utf8ByteMaskX) << 6)
             |  (UcdCP)(bytes[3] & _utf8ByteMaskX);
    default:
        return _decodingErrorCh;
    }
}

uint8_t utf8FromCP(UcdCP cp, Utf8Ch *outBuf) {
    if (!ucdIsCPValid(cp)) {
        return 0;
    }

    if (cp <= 0x7f) {
        outBuf[0] = (Utf8Ch)cp;
        return 1;
    } else if (cp <= 0x7ff) {
        outBuf[0] = 0xc0 | (Utf8Ch)(cp >> 6);
        outBuf[1] = 0x80 | (Utf8Ch)(cp & 0x3f);
        return 2;
    } else if (cp <= 0xffff) {
        outBuf[0] = 0xe0 | (Utf8Ch)(cp >> 12);
        outBuf[1] = 0x80 | (Utf8Ch)(cp >> 6 & 0x3f);
        outBuf[2] = 0x80 | (Utf8Ch)(cp & 0x3f);
        return 3;
    } else if (cp <= ucdCPMax) {
        outBuf[0] = 0xf0 | (Utf8Ch)(cp >> 18);
        outBuf[1] = 0x80 | (Utf8Ch)(cp >> 12 & 0x3f);
        outBuf[2] = 0x80 | (Utf8Ch)(cp >> 6 & 0x3f);
        outBuf[3] = 0x80 | (Utf8Ch)(cp & 0x3f);
        return 4;
    } else {
        return 0;
    }
}

bool utf8ChIsStart(Utf8Ch ch) {
    return ch < 0x80 || (ch >= 0xc2 && ch <= 0xf4);
}

bool utf8Check(const Utf8Ch *str, size_t strLen) {
    for (size_t strIdx = 0; strIdx < strLen;) {
        Utf8ChCheckResult res = _ucdCh8CheckSingle(str, strLen, strIdx);
        strIdx += res.advance;
        if (!res.isValid) {
            return false;
        }
    }
    return true;
}

uint8_t utf16ChRunLen(Utf16Ch firstCh) {
    return (
        1 + (!!(
            firstCh >= ucdHighSurrogateFirst
            && firstCh <= ucdHighSurrogateLast
        ))
    ) * (!!(
        firstCh < ucdLowSurrogateFirst || firstCh > ucdLowSurrogateLast
    ));
}

uint8_t utf16CPLen(UcdCP cp) {
    return (1 + (cp > 0xffff)) * ucdIsCPValid(cp);
}

Utf32Ch utf16ChToCP(const Utf16Ch *bytes) {
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
        return _decodingErrorCh;
    }

    return ((bytes[0] & 0x3ff) << 10) + (bytes[1] & 0x3ff) + 0x10000;
}

uint8_t utf16FromCP(UcdCP cp, Utf16Ch *outBuf) {
    if (!ucdIsCPValid(cp)) {
        return 0;
    }

    if (cp <= 0xffff) {
        outBuf[0] = (Utf16Ch)cp;
        return 1;
    } else if (cp <= ucdCPMax) {
        cp -= 0x10000;
        outBuf[0] = ucdHighSurrogateFirst | (Utf16Ch)(cp >> 10);
        outBuf[1] = ucdLowSurrogateFirst | (Utf16Ch)(cp & 0x3ff);
        return 2;
    } else {
        return 0;
    }
}
