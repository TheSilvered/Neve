#include "nv_test.h"
#include "nv_unicode.h"

void test_ucdCh8StrToCh16StrSizeU0000(void) {
    const UcdCh8 str[1] = { 0x00 };
    testAssert(ucdCh8StrToCh16Str(str, 1, NULL, 0) == 2);
}

void test_ucdCh8StrToCh16StrU0000(void) {
    UcdCh16 buf[2] = { 0xffff, 0xffff };
    const UcdCh8 str[1] = { 0x00 };
    testAssert(ucdCh8StrToCh16Str(str, 1, buf, 2) == 1);
    testAssert(buf[0] == 0x0000);
    testAssert(buf[1] == 0x0000);
}

void test_ucdCh8StrToCh16StrSizeU007f(void) {
    const UcdCh8 str[1] = { 0x7f };
    testAssert(ucdCh8StrToCh16Str(str, 1, NULL, 0) == 2);
}

void test_ucdCh8StrToCh16StrU007f(void) {
    UcdCh16 buf[2] = { 0xffff, 0xffff };
    const UcdCh8 str[1] = { 0x7f };

    testAssertRequire(ucdCh8StrToCh16Str(str, 1, buf, 2) == 1);
    testAssert(buf[0] == 0x007f);
}

void test_ucdCh8StrToCh16StrSizeU0000U007f(void) {
    const UcdCh8 str[2] = { 0x00, 0x7f };
    testAssert(ucdCh8StrToCh16Str(str, 2, NULL, 0) == 3);
}

void test_ucdCh8StrToCh16StrU0000U007f(void) {
    UcdCh16 buf[3] = { 0xffff, 0xffff, 0xffff };
    const UcdCh8 str[2] = { 0x00, 0x7f };

    testAssertRequire(ucdCh8StrToCh16Str(str, 2, buf, 3) == 2);
    testAssert(buf[0] == 0x0000);
    testAssert(buf[1] == 0x007f);
    testAssert(buf[2] == 0x0000);
}

void test_ucdCh8StrToCh16StrSizeU0080(void) {
    const UcdCh8 str[2] = { 0xc2, 0x80 };
    testAssert(ucdCh8StrToCh16Str(str, 2, NULL, 0) == 2);
}

void test_ucdCh8StrToCh16StrU0080(void) {
    UcdCh16 buf[2] = { 0xffff, 0xffff };
    const UcdCh8 str[2] = { 0xc2, 0x80 };

    testAssertRequire(ucdCh8StrToCh16Str(str, 2, buf, 2) == 1);
    testAssert(buf[0] == 0x0080);
    testAssert(buf[1] == 0x0000);
}

void test_ucdCh8StrToCh16StrSizeU07ff(void) {
    const UcdCh8 str[2] = { 0xdf, 0xbf };
    testAssert(ucdCh8StrToCh16Str(str, 2, NULL, 0) == 2);
}

void test_ucdCh8StrToCh16StrU07ff(void) {
    UcdCh16 buf[2] = { 0xffff, 0xffff };
    const UcdCh8 str[2] = { 0xdf, 0xbf };

    testAssertRequire(ucdCh8StrToCh16Str(str, 2, buf, 2) == 1);
    testAssert(buf[0] == 0x07ff);
    testAssert(buf[1] == 0x0000);
}

void test_ucdCh8StrToCh16StrSizeU0800(void) {
    const UcdCh8 str[3] = { 0xe0, 0xa0, 0x80 };
    testAssert(ucdCh8StrToCh16Str(str, 3, NULL, 0) == 2);
}

void test_ucdCh8StrToCh16StrU0800(void) {
    UcdCh16 buf[2] = { 0xffff, 0xffff };
    const UcdCh8 str[3] = { 0xe0, 0xa0, 0x80 };

    testAssertRequire(ucdCh8StrToCh16Str(str, 3, buf, 2) == 1);
    testAssert(buf[0] == 0x0800);
    testAssert(buf[1] == 0x0000);
}

void test_ucdCh8StrToCh16StrSizeUffff(void) {
    const UcdCh8 str[3] = { 0xef, 0xbf, 0xbf };
    testAssert(ucdCh8StrToCh16Str(str, 3, NULL, 0) == 2);
}

void test_ucdCh8StrToCh16StrUffff(void) {
    UcdCh16 buf[2] = { 0xffff, 0xffff };
    const UcdCh8 str[3] = { 0xef, 0xbf, 0xbf };

    testAssertRequire(ucdCh8StrToCh16Str(str, 3, buf, 2) == 1);
    testAssert(buf[0] == 0xffff);
    testAssert(buf[1] == 0x0000);
}

void test_ucdCh8StrToCh16StrSizeU10000(void) {
    const UcdCh8 str[4] = { 0xf0, 0x90, 0x80, 0x80 };
    testAssert(ucdCh8StrToCh16Str(str, 4, NULL, 0) == 3);
}

void test_ucdCh8StrToCh16StrU10000(void) {
    UcdCh16 buf[3] = { 0xffff, 0xffff };
    const UcdCh8 str[4] = { 0xf0, 0x90, 0x80, 0x80 };

    testAssertRequire(ucdCh8StrToCh16Str(str, 4, buf, 3) == 2);
    testAssert(buf[0] == ucdHighSurrogateFirst);
    testAssert(buf[1] == ucdLowSurrogateFirst);
    testAssert(buf[2] == 0x0000);
}

void test_ucdCh8StrToCh16StrSizeU10ffff(void) {
    const UcdCh8 str[4] = { 0xf4, 0x8f, 0xbf, 0xbf};
    testAssert(ucdCh8StrToCh16Str(str, 4, NULL, 0) == 3);
}

void test_ucdCh8StrToCh16StrU10ffff(void) {
    UcdCh16 buf[3] = { 0xffff, 0xffff, 0xffff };
    const UcdCh8 str[4] = { 0xf4, 0x8f, 0xbf, 0xbf};

    testAssertRequire(ucdCh8StrToCh16Str(str, 4, buf, 3) == 2);
    testAssert(buf[0] == ucdHighSurrogateLast);
    testAssert(buf[1] == ucdLowSurrogateLast);
    testAssert(buf[2] == 0x0000);
}

void test_ucdCh8StrToCh16StrSizeMix(void) {
    const UcdCh8 str[20] = {
        0x00, // U+0000
        0x7f, // U+007F
        0xc2, 0x80, // U+0080
        0xdf, 0xbf, // U+07FF
        0xe0, 0xa0, 0x80, // U+0800
        0xef, 0xbf, 0xbf, // U+FFFF
        0xf0, 0x90, 0x80, 0x80, // U+10000
        0xf4, 0x8f, 0xbf, 0xbf // U+10FFFF
    };
    testAssert(ucdCh8StrToCh16Str(str, 20, NULL, 0) == 11);
}

void test_ucdCh8StrToCh16StrMix(void) {
    UcdCh16 buf[11] = {
        0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
        0xffff, 0xffff
    };
    const UcdCh8 str[20] = {
        0x00, // U+0000
        0x7f, // U+007F
        0xc2, 0x80, // U+0080
        0xdf, 0xbf, // U+07FF
        0xe0, 0xa0, 0x80, // U+0800
        0xef, 0xbf, 0xbf, // U+FFFF
        0xf0, 0x90, 0x80, 0x80, // U+10000
        0xf4, 0x8f, 0xbf, 0xbf // U+10FFFF
    };
    testAssertRequire(ucdCh8StrToCh16Str(str, 20, buf, 11) == 10);

    testAssert(buf[ 0] == 0x0000);
    testAssert(buf[ 1] == 0x007f);
    testAssert(buf[ 2] == 0x0080);
    testAssert(buf[ 3] == 0x07ff);
    testAssert(buf[ 4] == 0x0800);
    testAssert(buf[ 5] == 0xffff);
    testAssert(buf[ 6] == ucdHighSurrogateFirst);
    testAssert(buf[ 7] == ucdLowSurrogateFirst);
    testAssert(buf[ 8] == ucdHighSurrogateLast);
    testAssert(buf[ 9] == ucdLowSurrogateLast);
    testAssert(buf[10] == 0x0000);
}

/* Below tests for invalid UTF8 inputs. */

void test_ucdCh8StrToCh16StrSizeHighSurrogateFirst(void) {
    // ucdHighSurrogateFirst encoded in UTF8
    const UcdCh8 str[3] = { 0xed, 0xa0, 0x80 };
    testAssert(ucdCh8StrToCh16Str(str, 3, NULL, 0) == 2);
}

void test_ucdCh8StrToCh16StrHighSurrogateFirst(void) {
    UcdCh16 buf[2] = { 0xffff, 0xffff };
    // ucdHighSurrogateFirst encoded in UTF8
    const UcdCh8 str[3] = { 0xed, 0xa0, 0x80 };
    // invalid UTF8, U+FFFD (invalid character) is encoded
    testAssertRequire(ucdCh8StrToCh16Str(str, 3, buf, 2) == 1);
    testAssert(buf[0] == 0xfffd);
    testAssert(buf[1] == 0x0000);
}

void test_ucdCh8StrToCh16StrSizeHighSurrogateLast(void) {
    // ucdHighSurrogateLast encoded in UTF8
    const UcdCh8 str[3] = { 0xed, 0xaf, 0xbf };
    testAssert(ucdCh8StrToCh16Str(str, 3, NULL, 0) == 2);
}

void test_ucdCh8StrToCh16StrHighSurrogateLast(void) {
    UcdCh16 buf[2] = { 0xffff, 0xffff };
    // ucdHighSurrogateLast encoded in UTF8
    const UcdCh8 str[3] = { 0xed, 0xaf, 0xbf };
    // invalid UTF8, U+FFFD (invalid character) is encoded
    testAssertRequire(ucdCh8StrToCh16Str(str, 3, buf, 2) == 1);
    testAssert(buf[0] == 0xfffd);
    testAssert(buf[1] == 0x0000);
}

void test_ucdCh8StrToCh16StrSizeLowSurrogateFirst(void) {
    // ucdLowSurrogateFirst encoded in UTF8
    const UcdCh8 str[3] = { 0xed, 0xb0, 0x80 };
    testAssert(ucdCh8StrToCh16Str(str, 3, NULL, 0) == 2);
}

void test_ucdCh8StrToCh16StrLowSurrogateFirst(void) {
    UcdCh16 buf[2] = { 0xffff, 0xffff };
    // ucdLowSurrogateFirst encoded in UTF8
    const UcdCh8 str[3] = { 0xed, 0xb0, 0x80 };
    // invalid UTF8, U+FFFD (invalid character) is encoded
    testAssertRequire(ucdCh8StrToCh16Str(str, 3, buf, 2) == 1);
    testAssert(buf[0] == 0xfffd);
    testAssert(buf[1] == 0x0000);
}

void test_ucdCh8StrToCh16StrSizeLowSurrogateLast(void) {
    // ucdLowSurrogateLast encoded in UTF8
    const UcdCh8 str[3] = { 0xed, 0xbf, 0xbf };
    testAssert(ucdCh8StrToCh16Str(str, 3, NULL, 0) == 2);
}

void test_ucdCh8StrToCh16StrLowSurrogateLast(void) {
    UcdCh16 buf[2] = { 0xffff, 0xffff };
    // ucdLowSurrogateLast encoded in UTF8
    const UcdCh8 str[3] = { 0xed, 0xbf, 0xbf };
    // invalid UTF8, U+FFFD (invalid character) is encoded
    testAssertRequire(ucdCh8StrToCh16Str(str, 3, buf, 2) == 1);
    testAssert(buf[0] == 0xfffd);
    testAssert(buf[1] == 0x0000);
}

void test_ucdCh8StrToCh16StrSizeInvalidByte(void) {
    const UcdCh8 str[1] = { 0xff };
    testAssert(ucdCh8StrToCh16Str(str, 1, NULL, 0) == 2);
}

void test_ucdCh8StrToCh16StrInvalidByte(void) {
    UcdCh16 buf[2] = { 0xffff, 0xffff };
    const UcdCh8 str[1] = { 0xff };
    // invalid UTF8, U+FFFD (invalid character) is encoded
    testAssertRequire(ucdCh8StrToCh16Str(str, 1, buf, 2) == 1);
    testAssert(buf[0] == 0xfffd);
    testAssert(buf[1] == 0x0000);
}

void test_ucdCh8StrToCh16StrSizeIncomplete2Sequence(void) {
    const UcdCh8 str[1] = { 0xc2 };
    testAssert(ucdCh8StrToCh16Str(str, 1, NULL, 0) == 2);
}

void test_ucdCh8StrToCh16StrIncomplete2Sequence(void) {
    UcdCh16 buf[2] = { 0xffff, 0xffff };
    const UcdCh8 str[1] = { 0xc2 };
    // invalid UTF8, U+FFFD (invalid character) is encoded
    testAssertRequire(ucdCh8StrToCh16Str(str, 1, buf, 2) == 1);
    testAssert(buf[0] == 0xfffd);
    testAssert(buf[1] == 0x0000);
}

void test_ucdCh8StrToCh16StrSizeIncorrect2Sequence(void) {
    const UcdCh8 str[2] = { 0xc2, 0x01 };
    testAssert(ucdCh8StrToCh16Str(str, 2, NULL, 0) == 3);
}

void test_ucdCh8StrToCh16StrIncorrect2Sequence(void) {
    UcdCh16 buf[3] = { 0xffff, 0xffff, 0xffff };
    const UcdCh8 str[2] = { 0xc2, 0x01 };
    // invalid UTF8, U+FFFD (invalid character) is encoded
    testAssertRequire(ucdCh8StrToCh16Str(str, 2, buf, 3) == 2);
    testAssert(buf[0] == 0xfffd);
    testAssert(buf[1] == 0x0001);
    testAssert(buf[2] == 0x0000);
}

void test_ucdCh8StrToCh16StrSizeIncomplete3Sequence(void) {
    const UcdCh8 str[2] = { 0xe0, 0x80 };
    testAssert(ucdCh8StrToCh16Str(str, 2, NULL, 0) == 3);
}

void test_ucdCh8StrToCh16StrIncomplete3Sequence(void) {
    UcdCh16 buf[3] = { 0xffff, 0xffff, 0xffff };
    const UcdCh8 str[2] = { 0xe0, 0x80 };
    // invalid UTF8, U+FFFD (invalid character) is encoded
    testAssertRequire(ucdCh8StrToCh16Str(str, 2, buf, 3) == 2);
    testAssert(buf[0] == 0xfffd);
    testAssert(buf[1] == 0xfffd);
    testAssert(buf[2] == 0x0000);
}

void test_ucdCh8StrToCh16StrSizeIncorrect3Sequence(void) {
    const UcdCh8 str[3] = { 0xe0, 0x80, 0x01 };
    testAssert(ucdCh8StrToCh16Str(str, 3, NULL, 0) == 4);
}

void test_ucdCh8StrToCh16StrIncorrect3Sequence(void) {
    UcdCh16 buf[4] = { 0xffff, 0xffff, 0xffff, 0xffff };
    const UcdCh8 str[3] = { 0xe0, 0x80, 0x01 };
    // invalid UTF8, U+FFFD (invalid character) is encoded
    testAssertRequire(ucdCh8StrToCh16Str(str, 3, buf, 4) == 3);
    testAssert(buf[0] == 0xfffd);
    testAssert(buf[1] == 0xfffd);
    testAssert(buf[2] == 0x0001);
    testAssert(buf[3] == 0x0000);
}

void test_ucdCh8StrToCh16StrSizeInvalid3Sequence(void) {
    const UcdCh8 str[3] = { 0xe0, 0x80, 0x80 };
    testAssert(ucdCh8StrToCh16Str(str, 3, NULL, 0) == 2);
}

void test_ucdCh8StrToCh16StrInvalid3Sequence(void) {
    UcdCh16 buf[2] = { 0xffff, 0xffff };
    const UcdCh8 str[3] = { 0xe0, 0x80, 0x80 };
    // invalid UTF8, U+FFFD (invalid character) is encoded
    testAssertRequire(ucdCh8StrToCh16Str(str, 3, buf, 2) == 1);
    testAssert(buf[0] == 0xfffd);
    testAssert(buf[1] == 0x0000);
}

void test_ucdCh8StrToCh16StrSizeIncomplete4Sequence(void) {
    const UcdCh8 str[3] = { 0xf0, 0x80, 0x80 };
    testAssert(ucdCh8StrToCh16Str(str, 3, NULL, 0) == 4);
}

void test_ucdCh8StrToCh16StrIncomplete4Sequence(void) {
    UcdCh16 buf[4] = { 0xffff, 0xffff, 0xffff, 0xffff };
    const UcdCh8 str[3] = { 0xf0, 0x80, 0x80 };
    // invalid UTF8, U+FFFD (invalid character) is encoded
    testAssertRequire(ucdCh8StrToCh16Str(str, 3, buf, 4) == 3);
    testAssert(buf[0] == 0xfffd);
    testAssert(buf[1] == 0xfffd);
    testAssert(buf[2] == 0xfffd);
    testAssert(buf[3] == 0x0000);
}

void test_ucdCh8StrToCh16StrSizeIncorrect4Sequence(void) {
    const UcdCh8 str[4] = { 0xf0, 0x80, 0x80, 0x01 };
    testAssert(ucdCh8StrToCh16Str(str, 4, NULL, 0) == 5);
}

void test_ucdCh8StrToCh16StrIncorrect4Sequence(void) {
    UcdCh16 buf[5] = { 0xffff, 0xffff, 0xffff, 0xffff, 0xffff };
    const UcdCh8 str[4] = { 0xf0, 0x80, 0x80, 0x01 };
    // invalid UTF8, U+FFFD (invalid character) is encoded
    testAssertRequire(ucdCh8StrToCh16Str(str, 4, buf, 5) == 4);
    testAssert(buf[0] == 0xfffd);
    testAssert(buf[1] == 0xfffd);
    testAssert(buf[2] == 0xfffd);
    testAssert(buf[3] == 0x0001);
    testAssert(buf[4] == 0x0000);
}

void test_ucdCh8StrToCh16StrSizeInvalid4Sequence(void) {
    const UcdCh8 str[4] = { 0xf0, 0x80, 0x80, 0x80 };
    testAssert(ucdCh8StrToCh16Str(str, 4, NULL, 0) == 2);
}

void test_ucdCh8StrToCh16StrInvalid4Sequence(void) {
    UcdCh16 buf[2] = { 0xffff, 0xffff };
    const UcdCh8 str[4] = { 0xf0, 0x80, 0x80, 0x80 };
    // invalid UTF8, U+FFFD (invalid character) is encoded
    testAssertRequire(ucdCh8StrToCh16Str(str, 4, buf, 2) == 1);
    testAssert(buf[0] == 0xfffd);
    testAssert(buf[1] == 0x0000);
}

testList(
    testMake(test_ucdCh8StrToCh16StrSizeU0000),
    testMake(test_ucdCh8StrToCh16StrU0000),
    testMake(test_ucdCh8StrToCh16StrSizeU007f),
    testMake(test_ucdCh8StrToCh16StrU007f),
    testMake(test_ucdCh8StrToCh16StrSizeU0000U007f),
    testMake(test_ucdCh8StrToCh16StrU0000U007f),
    testMake(test_ucdCh8StrToCh16StrSizeU0080),
    testMake(test_ucdCh8StrToCh16StrU0080),
    testMake(test_ucdCh8StrToCh16StrSizeU07ff),
    testMake(test_ucdCh8StrToCh16StrU07ff),
    testMake(test_ucdCh8StrToCh16StrSizeU0800),
    testMake(test_ucdCh8StrToCh16StrU0800),
    testMake(test_ucdCh8StrToCh16StrSizeUffff),
    testMake(test_ucdCh8StrToCh16StrUffff),
    testMake(test_ucdCh8StrToCh16StrSizeU10000),
    testMake(test_ucdCh8StrToCh16StrU10000),
    testMake(test_ucdCh8StrToCh16StrSizeMix),
    testMake(test_ucdCh8StrToCh16StrMix),
    testMake(test_ucdCh8StrToCh16StrSizeHighSurrogateFirst),
    testMake(test_ucdCh8StrToCh16StrHighSurrogateFirst),
    testMake(test_ucdCh8StrToCh16StrSizeHighSurrogateLast),
    testMake(test_ucdCh8StrToCh16StrHighSurrogateFirst),
    testMake(test_ucdCh8StrToCh16StrSizeLowSurrogateFirst),
    testMake(test_ucdCh8StrToCh16StrLowSurrogateFirst),
    testMake(test_ucdCh8StrToCh16StrSizeLowSurrogateLast),
    testMake(test_ucdCh8StrToCh16StrLowSurrogateLast),
    testMake(test_ucdCh8StrToCh16StrSizeInvalidByte),
    testMake(test_ucdCh8StrToCh16StrInvalidByte),
    testMake(test_ucdCh8StrToCh16StrSizeIncomplete2Sequence),
    testMake(test_ucdCh8StrToCh16StrIncomplete2Sequence),
    testMake(test_ucdCh8StrToCh16StrSizeIncorrect2Sequence),
    testMake(test_ucdCh8StrToCh16StrIncorrect2Sequence),
    testMake(test_ucdCh8StrToCh16StrSizeIncomplete3Sequence),
    testMake(test_ucdCh8StrToCh16StrIncomplete3Sequence),
    testMake(test_ucdCh8StrToCh16StrSizeIncorrect3Sequence),
    testMake(test_ucdCh8StrToCh16StrIncorrect3Sequence),
    testMake(test_ucdCh8StrToCh16StrSizeInvalid3Sequence),
    testMake(test_ucdCh8StrToCh16StrInvalid3Sequence),
    testMake(test_ucdCh8StrToCh16StrSizeIncomplete4Sequence),
    testMake(test_ucdCh8StrToCh16StrIncomplete4Sequence),
    testMake(test_ucdCh8StrToCh16StrSizeIncorrect4Sequence),
    testMake(test_ucdCh8StrToCh16StrIncorrect4Sequence),
    testMake(test_ucdCh8StrToCh16StrSizeInvalid4Sequence),
    testMake(test_ucdCh8StrToCh16StrInvalid4Sequence),
);
