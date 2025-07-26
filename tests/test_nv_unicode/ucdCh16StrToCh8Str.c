#include "nv_test.h"
#include "nv_unicode.h"

void test_ucdCh16StrToCh8StrSizeU0000(void) {
    const UcdCh16 str[1] = { 0x0000 };
    testAssert(ucdCh16StrToCh8Str(str, 1, NULL, 0) == 2);
}

void test_ucdCh16StrToCh8StrU0000(void) {
    UcdCh8 buf[2] = { 0xff, 0xff };
    const UcdCh16 str[1] = { 0x0000 };

    testAssertRequire(ucdCh16StrToCh8Str(str, 1, buf, 2) == 1);
    testAssert(buf[0] == 0x00);
    testAssert(buf[1] == 0x00);
}

void test_ucdCh16StrToCh8StrSizeU007f(void) {
    const UcdCh16 str[1] = { 0x007f };
    testAssert(ucdCh16StrToCh8Str(str, 1, NULL, 0) == 2);
}

void test_ucdCh16StrToCh8StrU007f(void) {
    UcdCh8 buf[2] = { 0xff, 0xff };
    const UcdCh16 str[1] = { 0x007f };

    testAssertRequire(ucdCh16StrToCh8Str(str, 1, buf, 2) == 1);
    testAssert(buf[0] == 0x7f);
    testAssert(buf[1] == 0x00);
}

void test_ucdCh16StrToCh8StrSizeU0000U007f(void) {
    const UcdCh16 str[2] = { 0x0000, 0x007f };
    testAssert(ucdCh16StrToCh8Str(str, 2, NULL, 0) == 3);
}

void test_ucdCh16StrToCh8StrU0000U007f(void) {
    UcdCh8 buf[3] = { 0xff, 0xff, 0xff };
    const UcdCh16 str[2] = { 0x0000, 0x007f };

    testAssertRequire(ucdCh16StrToCh8Str(str, 2, buf, 3) == 2);
    testAssert(buf[0] == 0x00);
    testAssert(buf[1] == 0x7f);
    testAssert(buf[2] == 0x00);
}

void test_ucdCh16StrToCh8StrSizeU0080(void) {
    const UcdCh16 str[1] = { 0x0080 };
    testAssert(ucdCh16StrToCh8Str(str, 1, NULL, 0) == 3);
}

void test_ucdCh16StrToCh8StrU0080(void) {
    UcdCh8 buf[3] = { 0xff, 0xff, 0xff };
    const UcdCh16 str[1] = { 0x0080 };

    testAssertRequire(ucdCh16StrToCh8Str(str, 1, buf, 3) == 2);
    testAssert(buf[0] == 0xc2);
    testAssert(buf[1] == 0x80);
    testAssert(buf[2] == 0x00);
}

void test_ucdCh16StrToCh8StrSizeU07ff(void) {
    const UcdCh16 str[1] = { 0x07ff };
    testAssert(ucdCh16StrToCh8Str(str, 1, NULL, 0) == 3);
}

void test_ucdCh16StrToCh8StrU07ff(void) {
    UcdCh8 buf[3] = { 0xff, 0xff, 0xff };
    const UcdCh16 str[1] = { 0x07ff };

    testAssertRequire(ucdCh16StrToCh8Str(str, 1, buf, 3) == 2);
    testAssert(buf[0] == 0xdf);
    testAssert(buf[1] == 0xbf);
    testAssert(buf[2] == 0x00);
}

void test_ucdCh16StrToCh8StrSizeU0800(void) {
    const UcdCh16 str[1] = { 0x0800 };
    testAssert(ucdCh16StrToCh8Str(str, 1, NULL, 0) == 4);
}

void test_ucdCh16StrToCh8StrU0800(void) {
    UcdCh8 buf[4] = { 0xff, 0xff, 0xff, 0xff };
    const UcdCh16 str[1] = { 0x0800 };

    testAssertRequire(ucdCh16StrToCh8Str(str, 1, buf, 4) == 3);
    testAssert(buf[0] == 0xe0);
    testAssert(buf[1] == 0xa0);
    testAssert(buf[2] == 0x80);
    testAssert(buf[3] == 0x00);
}

void test_ucdCh16StrToCh8StrSizeUffff(void) {
    const UcdCh16 str[1] = { 0xffff };
    testAssert(ucdCh16StrToCh8Str(str, 1, NULL, 0) == 4);
}

void test_ucdCh16StrToCh8StrUffff(void) {
    UcdCh8 buf[4] = { 0xff, 0xff, 0xff, 0xff };
    const UcdCh16 str[1] = { 0xffff };

    testAssertRequire(ucdCh16StrToCh8Str(str, 1, buf, 4) == 3);
    testAssert(buf[0] == 0xef);
    testAssert(buf[1] == 0xbf);
    testAssert(buf[2] == 0xbf);
    testAssert(buf[3] == 0x00);
}

void test_ucdCh16StrToCh8StrSizeU10000(void) {
    const UcdCh16 str[2] = {
        ucdHighSurrogateFirst,
        ucdLowSurrogateFirst
    };
    testAssert(ucdCh16StrToCh8Str(str, 2, NULL, 0) == 5);
}

void test_ucdCh16StrToCh8StrU10000(void) {
    UcdCh8 buf[5] = { 0xff, 0xff, 0xff, 0xff, 0xff };
    const UcdCh16 str[2] = {
        ucdHighSurrogateFirst,
        ucdLowSurrogateFirst
    };

    testAssertRequire(ucdCh16StrToCh8Str(str, 2, buf, 5) == 4);
    testAssert(buf[0] == 0xf0);
    testAssert(buf[1] == 0x90);
    testAssert(buf[2] == 0x80);
    testAssert(buf[3] == 0x80);
    testAssert(buf[4] == 0x00);
}

void test_ucdCh16StrToCh8StrSizeU10ffff(void) {
    const UcdCh16 str[2] = {
        ucdHighSurrogateLast,
        ucdLowSurrogateLast
    };
    testAssert(ucdCh16StrToCh8Str(str, 2, NULL, 0) == 5);
}

void test_ucdCh16StrToCh8StrU10ffff(void) {
    UcdCh8 buf[5] = { 0xff, 0xff, 0xff, 0xff, 0xff };
    const UcdCh16 str[2] = {
        ucdHighSurrogateLast,
        ucdLowSurrogateLast
    };

    testAssertRequire(ucdCh16StrToCh8Str(str, 2, buf, 5) == 4);
    testAssert(buf[0] == 0xf4);
    testAssert(buf[1] == 0x8f);
    testAssert(buf[2] == 0xbf);
    testAssert(buf[3] == 0xbf);
    testAssert(buf[4] == 0x00);
}

void test_ucdCh16StrToCh8StrSizeMix(void) {
    const UcdCh16 str[10] = {
        0x0000,
        0x007f,
        0x0080,
        0x07ff,
        0x0800,
        0xffff,
        ucdHighSurrogateFirst, ucdLowSurrogateFirst,
        ucdHighSurrogateLast, ucdLowSurrogateLast
    };
    testAssert(ucdCh16StrToCh8Str(str, 10, NULL, 0) == 21);
}

void test_ucdCh16StrToCh8StrMix(void) {
    UcdCh8 buf[21] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    const UcdCh16 str[10] = {
        0x0000,
        0x007f,
        0x0080,
        0x07ff,
        0x0800,
        0xffff,
        ucdHighSurrogateFirst, ucdLowSurrogateFirst,
        ucdHighSurrogateLast, ucdLowSurrogateLast
    };
    testAssertRequire(ucdCh16StrToCh8Str(str, 10, buf, 21) == 20);
    testAssert(buf[ 0] == 0x00);
    testAssert(buf[ 1] == 0x7f);
    testAssert(buf[ 2] == 0xc2);
    testAssert(buf[ 3] == 0x80);
    testAssert(buf[ 4] == 0xdf);
    testAssert(buf[ 5] == 0xbf);
    testAssert(buf[ 6] == 0xe0);
    testAssert(buf[ 7] == 0xa0);
    testAssert(buf[ 8] == 0x80);
    testAssert(buf[ 9] == 0xef);
    testAssert(buf[10] == 0xbf);
    testAssert(buf[11] == 0xbf);
    testAssert(buf[12] == 0xf0);
    testAssert(buf[13] == 0x90);
    testAssert(buf[14] == 0x80);
    testAssert(buf[15] == 0x80);
    testAssert(buf[16] == 0xf4);
    testAssert(buf[17] == 0x8f);
    testAssert(buf[18] == 0xbf);
    testAssert(buf[19] == 0xbf);
    testAssert(buf[20] == 0x00);
}

/* Below tests for invalid UTF16 inputs. */

void test_ucdCh16StrToCh8StrSizeHighSurrogateFirst(void) {
    const UcdCh16 str[1] = { ucdHighSurrogateFirst };
    testAssert(ucdCh16StrToCh8Str(str, 1, NULL, 0) == 4);
}

void test_ucdCh16StrToCh8StrHighSurrogateFirst(void) {
    UcdCh8 buf[4] = { 0xff, 0xff, 0xff, 0xff };
    const UcdCh16 str[1] = { ucdHighSurrogateFirst };
    // invalid UTF16, U+FFFD (invalid character) is encoded
    testAssertRequire(ucdCh16StrToCh8Str(str, 1, buf, 4) == 3);
    testAssert(buf[0] == 0xef);
    testAssert(buf[1] == 0xbf);
    testAssert(buf[2] == 0xbd);
    testAssert(buf[3] == 0x00);
}

void test_ucdCh16StrToCh8StrSizeHighSurrogateLast(void) {
    const UcdCh16 str[1] = { ucdHighSurrogateLast };
    testAssert(ucdCh16StrToCh8Str(str, 1, NULL, 0) == 4);
}

void test_ucdCh16StrToCh8StrHighSurrogateLast(void) {
    UcdCh8 buf[4] = { 0xff, 0xff, 0xff, 0xff };
    const UcdCh16 str[1] = { ucdHighSurrogateLast };
    // invalid UTF16, U+FFFD (invalid character) is encoded
    testAssertRequire(ucdCh16StrToCh8Str(str, 1, buf, 4) == 3);
    testAssert(buf[0] == 0xef);
    testAssert(buf[1] == 0xbf);
    testAssert(buf[2] == 0xbd);
    testAssert(buf[3] == 0x00);
}

void test_ucdCh16StrToCh8StrSizeLowSurrogateFirst(void) {
    const UcdCh16 str[1] = { ucdLowSurrogateFirst };
    testAssert(ucdCh16StrToCh8Str(str, 1, NULL, 0) == 4);
}

void test_ucdCh16StrToCh8StrLowSurrogateFirst(void) {
    UcdCh8 buf[4] = { 0xff, 0xff, 0xff, 0xff };
    const UcdCh16 str[1] = { ucdLowSurrogateFirst };
    // invalid UTF16, U+FFFD (invalid character) is encoded
    testAssertRequire(ucdCh16StrToCh8Str(str, 1, buf, 4) == 3);
    testAssert(buf[0] == 0xef);
    testAssert(buf[1] == 0xbf);
    testAssert(buf[2] == 0xbd);
    testAssert(buf[3] == 0x00);
}

void test_ucdCh16StrToCh8StrSizeLowSurrogateLast(void) {
    const UcdCh16 str[1] = { ucdLowSurrogateLast };
    testAssert(ucdCh16StrToCh8Str(str, 1, NULL, 0) == 4);
}

void test_ucdCh16StrToCh8StrLowSurrogateLast(void) {
    UcdCh8 buf[4] = { 0xff, 0xff, 0xff, 0xff };
    const UcdCh16 str[1] = { ucdLowSurrogateLast };
    // invalid UTF16, U+FFFD (invalid character) is encoded
    testAssertRequire(ucdCh16StrToCh8Str(str, 1, buf, 4) == 3);
    testAssert(buf[0] == 0xef);
    testAssert(buf[1] == 0xbf);
    testAssert(buf[2] == 0xbd);
    testAssert(buf[3] == 0x00);
}

void test_ucdCh16StrToCh8StrSizeIncompleteSurrogate(void) {
    const UcdCh16 str[2] = { ucdHighSurrogateFirst, 1 };
    testAssert(ucdCh16StrToCh8Str(str, 2, NULL, 0) == 5);
}

void test_ucdCh16StrToCh8StrIncompleteSurrogate(void) {
    UcdCh8 buf[5] = { 0xff, 0xff, 0xff, 0xff };
    const UcdCh16 str[2] = { ucdHighSurrogateFirst, 1 };
    // invalid UTF16, U+FFFD (invalid character) is encoded
    testAssertRequire(ucdCh16StrToCh8Str(str, 2, buf, 5) == 4);
    testAssert(buf[0] == 0xef);
    testAssert(buf[1] == 0xbf);
    testAssert(buf[2] == 0xbd);
    testAssert(buf[3] == 0x01);
    testAssert(buf[4] == 0x00);
}

testList(
    testMake(test_ucdCh16StrToCh8StrSizeU0000),
    testMake(test_ucdCh16StrToCh8StrU0000),
    testMake(test_ucdCh16StrToCh8StrSizeU007f),
    testMake(test_ucdCh16StrToCh8StrU007f),
    testMake(test_ucdCh16StrToCh8StrSizeU0000U007f),
    testMake(test_ucdCh16StrToCh8StrU0000U007f),
    testMake(test_ucdCh16StrToCh8StrSizeU0080),
    testMake(test_ucdCh16StrToCh8StrU0080),
    testMake(test_ucdCh16StrToCh8StrSizeU07ff),
    testMake(test_ucdCh16StrToCh8StrU07ff),
    testMake(test_ucdCh16StrToCh8StrSizeU0800),
    testMake(test_ucdCh16StrToCh8StrU0800),
    testMake(test_ucdCh16StrToCh8StrSizeUffff),
    testMake(test_ucdCh16StrToCh8StrUffff),
    testMake(test_ucdCh16StrToCh8StrSizeU10000),
    testMake(test_ucdCh16StrToCh8StrU10000),
    testMake(test_ucdCh16StrToCh8StrSizeU10ffff),
    testMake(test_ucdCh16StrToCh8StrU10ffff),
    testMake(test_ucdCh16StrToCh8StrSizeMix),
    testMake(test_ucdCh16StrToCh8StrMix),
    testMake(test_ucdCh16StrToCh8StrSizeHighSurrogateFirst),
    testMake(test_ucdCh16StrToCh8StrHighSurrogateFirst),
    testMake(test_ucdCh16StrToCh8StrSizeHighSurrogateLast),
    testMake(test_ucdCh16StrToCh8StrHighSurrogateLast),
    testMake(test_ucdCh16StrToCh8StrSizeLowSurrogateFirst),
    testMake(test_ucdCh16StrToCh8StrLowSurrogateFirst),
    testMake(test_ucdCh16StrToCh8StrSizeLowSurrogateLast),
    testMake(test_ucdCh16StrToCh8StrLowSurrogateLast),
    testMake(test_ucdCh16StrToCh8StrSizeIncompleteSurrogate),
    testMake(test_ucdCh16StrToCh8StrIncompleteSurrogate)
)
