#include <assert.h>

#include "nv_unicode.h"
#include "nv_udb.h"
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
#define UTF8_ByteMask2 0x1f // 0b00011111
// Mask for the data of the first byte of a 3-byte character sequence
#define UTF8_ByteMask3 0x0f // 0b00001111
// Mask for the data of the first byte of a 4-byte character sequence
#define UTF8_ByteMask4 0x07 // 0b00000111
// Mask for the data of following bytes in a character sequence
#define UTF8_ByteMaskX 0x3f // 0b00111111

#define _decodingErrorCh 0xfffd

typedef struct UcdCh8CheckResult {
    UcdCP cp;
    uint8_t advance;
    bool isValid;
} UcdCh8CheckResult;

static UcdCh8CheckResult _ucdCh8CheckSingle(
    const UcdCh8 *str,
    size_t len,
    size_t idx
) {
    UcdCh8 ch8 = str[idx++];
    UcdCP ch = 0;
    uint8_t advance = 1;
    UcdCP cp = _decodingErrorCh;
    bool isValid = false;

    switch (ucdCh8RunLen(ch8)) {
    case 0:
        break;
    case 1:
        cp = ch8;
        isValid = true;
        break;
    case 2:
        if (idx == len || (str[idx] & (~UTF8_ByteMaskX)) != 0x80) {
            break;
        }
        advance = 2;

        ch = ((ch8        & UTF8_ByteMask2) << 6);
        ch |= (str[idx++] & UTF8_ByteMaskX);
        if (ch >= 0x80) {
            cp = ch;
            isValid = true;
        }
        break;
    case 3:
        if (
            idx >= len - 1
            || (str[idx] & (~UTF8_ByteMaskX)) != 0x80
            || (str[idx + 1] & (~UTF8_ByteMaskX)) != 0x80
        ) {
            break;
        }
        advance = 3;

        ch  = ((ch8        & UTF8_ByteMask3) << 12);
        ch |= ((str[idx++] & UTF8_ByteMaskX) << 6);
        ch |=  (str[idx++] & UTF8_ByteMaskX);
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
            || (str[idx] & (~UTF8_ByteMaskX)) != 0x80
            || (str[idx + 1] & (~UTF8_ByteMaskX)) != 0x80
            || (str[idx + 2] & (~UTF8_ByteMaskX)) != 0x80
        ) {
            break;
        }
        advance = 4;

        ch  = ((ch8        & UTF8_ByteMask4) << 18);
        ch |= ((str[idx++] & UTF8_ByteMaskX) << 12);
        ch |= ((str[idx++] & UTF8_ByteMaskX) << 6);
        ch |=  (str[idx++] & UTF8_ByteMaskX);
        if (ch >= 0x10000 && ch <= ucdCPMax) {
            cp = ch;
            isValid = true;
        }
        break;
    default:
        nvUnreachable;
        break;
    }

    return (UcdCh8CheckResult) {
        .advance = advance,
        .cp = cp,
        .isValid = isValid
    };
}

bool ucdIsCPValid(UcdCP cp) {
    return cp <= ucdCPMax
        && cp >= 0
        && (cp < ucdHighSurrogateFirst || cp > ucdLowSurrogateLast);
}

uint8_t ucdCPWidth(UcdCP cp, uint8_t tabStop, size_t currWidth) {
    if (cp == '\t') {
        return tabStop - (currWidth % tabStop);
    }

    // Short path for ASCII printable characters
    if (cp >= ' ' && cp <= '~') {
        return 1;
    } else {
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
            nvUnreachable;
        }
    }
}

bool ucdIsCPAlphanumeric(UcdCP cp) {
    // Ascii shortcut
    if (
        (cp >= '0' && cp <= '9')
        || (cp >= 'a' && cp <= 'z')
        || (cp >= 'A' && cp <= 'Z')
    ) {
        return true;
    } else if (!ucdIsCPValid(cp) || cp < 0x80) {
        return false;
    }

    UdbCPInfo info = udbGetCPInfo(cp);
    return UdbMajorCategory(info.category) == UdbCategory_L
        || UdbMajorCategory(info.category) == UdbCategory_N;
}

bool ucdIsCPWhiteSpace(UcdCP cp) {
    switch (cp) {
    case 0x0009:
    case 0x000A:
    case 0x000B:
    case 0x000C:
    case 0x000D:
    case 0x0020:
    case 0x0085:
    case 0x00A0:
    case 0x1680:
    case 0x2000:
    case 0x2001:
    case 0x2002:
    case 0x2003:
    case 0x2004:
    case 0x2005:
    case 0x2006:
    case 0x2007:
    case 0x2008:
    case 0x2009:
    case 0x200A:
    case 0x2028:
    case 0x2029:
    case 0x202F:
    case 0x205F:
    case 0x3000:
        return true;
    }
    return false;
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
        } else if (ch <= ucdCPMax && bufLen - bufIdx > 4) {
            buf[bufIdx++] = 0xf0 | (UcdCh8)(ch >> 18);
            buf[bufIdx++] = 0x80 | (UcdCh8)(ch >> 12 & 0x3f);
            buf[bufIdx++] = 0x80 | (UcdCh8)(ch >> 6 & 0x3f);
            buf[bufIdx++] = 0x80 | (UcdCh8)(ch & 0x3f);
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

size_t ucdCh8StrToCh16Str(
    const UcdCh8 *str, size_t strLen,
    UcdCh16 *buf, size_t bufLen
) {
    size_t bufIdx = 0;
    for (size_t strIdx = 0; strIdx < strLen;) {
        UcdCh8CheckResult res = _ucdCh8CheckSingle(str, strLen, strIdx);
        strIdx += res.advance;
        UcdCP ch = res.cp;

        // all lenth checks use '>' instead of '>=' because an extra NUL
        // character is added at the end

        if (buf == NULL) {
            bufIdx += ucdCh16CPLen(ch);
        } else if (ch <= 0xffff && bufLen - bufIdx > 1) {
            buf[bufIdx++] = (UcdCh16)ch;
        } else if (ch <= ucdCPMax && bufLen - bufIdx > 2) {
            ch -= 0x10000;
            buf[bufIdx++] = ucdHighSurrogateFirst | (UcdCh16)(ch >> 10);
            buf[bufIdx++] = ucdLowSurrogateFirst | (UcdCh16)(ch & 0x3ff);
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

uint8_t ucdCh8RunLen(UcdCh8 byte0) {
    return (byte0 < 0x80)
         + 2*((byte0 & ~UTF8_ByteMask2) == 0xc0)*(byte0 >= 0xc2)
         + 3*((byte0 & ~UTF8_ByteMask3) == 0xe0)
         + 4*((byte0 & ~UTF8_ByteMask4) == 0xf0)*(byte0 <= 0xf4);
}

uint8_t ucdCh8CPLen(UcdCP ch) {
    return (ch >= 0 && ch < 0x80)
         + 2*(!!(ch >= 0x80 && ch < 0x800))
         + 3*(!!(ch >= 0x800 && ch < ucdHighSurrogateFirst))
         + 3*(!!(ch > ucdLowSurrogateLast && ch < 0x10000))
         + 4*(!!(ch >= 0x10000 && ch <= ucdCPMax));
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
        return _decodingErrorCh;
    }
}

uint8_t ucdCh8FromCP(UcdCP cp, UcdCh8 *outBuf) {
    if (!ucdIsCPValid(cp)) {
        return 0;
    }

    if (cp <= 0x7f) {
        outBuf[0] = (UcdCh8)cp;
        return 1;
    } else if (cp <= 0x7ff) {
        outBuf[0] = 0xc0 | (UcdCh8)(cp >> 6);
        outBuf[1] = 0x80 | (UcdCh8)(cp & 0x3f);
        return 2;
    } else if (cp <= 0xffff) {
        outBuf[0] = 0xe0 | (UcdCh8)(cp >> 12);
        outBuf[1] = 0x80 | (UcdCh8)(cp >> 6 & 0x3f);
        outBuf[2] = 0x80 | (UcdCh8)(cp & 0x3f);
        return 3;
    } else if (cp <= ucdCPMax) {
        outBuf[0] = 0xf0 | (UcdCh8)(cp >> 18);
        outBuf[1] = 0x80 | (UcdCh8)(cp >> 12 & 0x3f);
        outBuf[2] = 0x80 | (UcdCh8)(cp >> 6 & 0x3f);
        outBuf[3] = 0x80 | (UcdCh8)(cp & 0x3f);
        return 4;
    } else {
        return 0;
    }
}

bool ucdCh8IsStart(UcdCh8 ch) {
    return ch < 0x80 || (ch >= 0xc2 && ch <= 0xf4);
}

bool ucdCh8Check(const UcdCh8 *str, size_t strLen) {
    for (size_t strIdx = 0; strIdx < strLen;) {
        UcdCh8CheckResult res = _ucdCh8CheckSingle(str, strLen, strIdx);
        strIdx += res.advance;
        if (!res.isValid) {
            return false;
        }
    }
    return true;
}

uint8_t ucdCh16RunLen(UcdCh16 firstCh) {
    return (
        1 + (!!(
            firstCh >= ucdHighSurrogateFirst
            && firstCh <= ucdHighSurrogateLast
        ))
    ) * (!!(
        firstCh < ucdLowSurrogateFirst || firstCh > ucdLowSurrogateLast
    ));
}

uint8_t ucdCh16CPLen(UcdCP cp) {
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
        return _decodingErrorCh;
    }

    return ((bytes[0] & 0x3ff) << 10) + (bytes[1] & 0x3ff) + 0x10000;
}

uint8_t ucdCh16FromCP(UcdCP cp, UcdCh16 *outBuf) {
    if (!ucdIsCPValid(cp)) {
        return 0;
    }

    if (cp <= 0xffff) {
        outBuf[0] = (UcdCh16)cp;
        return 1;
    } else if (cp <= ucdCPMax) {
        cp -= 0x10000;
        outBuf[0] = ucdHighSurrogateFirst | (UcdCh16)(cp >> 10);
        outBuf[1] = ucdLowSurrogateFirst | (UcdCh16)(cp & 0x3ff);
        return 2;
    } else {
        return 0;
    }
}
