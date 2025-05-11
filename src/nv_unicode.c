#include "nv_unicode.h"

size_t ucdUTF16ToUTF8(
    UcdCh16 *str, size_t strLen,
    UcdCh8  *buf, size_t bufLen
) {
    size_t bufIdx = 0;
    for (size_t strIdx = 0; strIdx < strLen;) {
        UcdCh16 wch = str[strIdx++];
        UcdCh32 unicodeCP = 0;
        if (wch < 0xd800 || wch > 0xdfff) {
            unicodeCP = wch;
        } else {
            if (strIdx == strLen) {
                break;
            }
            unicodeCP = ((wch & 0x3ff) << 10)
                      + (str[strIdx++] & 0x3ff)
                      + 0x10000;
        }

        // ignore invalid characters
        if (unicodeCP > 0x10ffff) {
            continue;
        }

        // all lenth checks use '>' instead of '>=' because an extra NUL
        // character is added at the end

        if (unicodeCP <= 0x7f && bufLen - bufIdx > 1) {
            buf[bufIdx++] = (char)unicodeCP;
        } else if (unicodeCP <= 0x7ff && bufLen - bufIdx > 2) {
            buf[bufIdx++] = 0b11000000 | (char)(unicodeCP >> 6);
            buf[bufIdx++] = 0b10000000 | (char)(unicodeCP & 0x3f);
        } else if (unicodeCP <= 0xffff && bufLen - bufIdx > 3) {
            buf[bufIdx++] = 0b11100000 | (char)(unicodeCP >> 12);
            buf[bufIdx++] = 0b10000000 | (char)(unicodeCP >> 6 & 0x3f);
            buf[bufIdx++] = 0b10000000 | (char)(unicodeCP & 0x3f);
        } else if (unicodeCP <= 0x10ffff && bufLen - bufIdx > 4) {
            buf[bufIdx++] = 0b11110000 | (char)(unicodeCP >> 18);
            buf[bufIdx++] = 0b10000000 | (char)(unicodeCP >> 12 & 0x3f);
            buf[bufIdx++] = 0b10000000 | (char)(unicodeCP >> 6 & 0x3f);
            buf[bufIdx++] = 0b10000000 | (char)(unicodeCP & 0x3f);
        } else
            break;
    }
    buf[bufIdx] = '\0';
    return bufIdx;
}
