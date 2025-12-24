#include "nv_test.h"
#include "unicode/nv_utf.h"

void test_utf8ToUtf16SizeU0000(void) {
    const Utf8Ch str[1] = { 0x00 };
    testAssert(utf8ToUtf16(str, 1, NULL, 0) == 2);
}

void test_utf8ToUtf16U0000(void) {
    Utf16Ch buf[2] = { 0xffff, 0xffff };
    const Utf8Ch str[1] = { 0x00 };
    testAssert(utf8ToUtf16(str, 1, buf, 2) == 1);
    testAssert(buf[0] == 0x0000);
    testAssert(buf[1] == 0x0000);
}

void test_utf8ToUtf16SizeU007f(void) {
    const Utf8Ch str[1] = { 0x7f };
    testAssert(utf8ToUtf16(str, 1, NULL, 0) == 2);
}

void test_utf8ToUtf16U007f(void) {
    Utf16Ch buf[2] = { 0xffff, 0xffff };
    const Utf8Ch str[1] = { 0x7f };

    testAssertRequire(utf8ToUtf16(str, 1, buf, 2) == 1);
    testAssert(buf[0] == 0x007f);
}

void test_utf8ToUtf16SizeU0000U007f(void) {
    const Utf8Ch str[2] = { 0x00, 0x7f };
    testAssert(utf8ToUtf16(str, 2, NULL, 0) == 3);
}

void test_utf8ToUtf16U0000U007f(void) {
    Utf16Ch buf[3] = { 0xffff, 0xffff, 0xffff };
    const Utf8Ch str[2] = { 0x00, 0x7f };

    testAssertRequire(utf8ToUtf16(str, 2, buf, 3) == 2);
    testAssert(buf[0] == 0x0000);
    testAssert(buf[1] == 0x007f);
    testAssert(buf[2] == 0x0000);
}

void test_utf8ToUtf16SizeU0080(void) {
    const Utf8Ch str[2] = { 0xc2, 0x80 };
    testAssert(utf8ToUtf16(str, 2, NULL, 0) == 2);
}

void test_utf8ToUtf16U0080(void) {
    Utf16Ch buf[2] = { 0xffff, 0xffff };
    const Utf8Ch str[2] = { 0xc2, 0x80 };

    testAssertRequire(utf8ToUtf16(str, 2, buf, 2) == 1);
    testAssert(buf[0] == 0x0080);
    testAssert(buf[1] == 0x0000);
}

void test_utf8ToUtf16SizeU07ff(void) {
    const Utf8Ch str[2] = { 0xdf, 0xbf };
    testAssert(utf8ToUtf16(str, 2, NULL, 0) == 2);
}

void test_utf8ToUtf16U07ff(void) {
    Utf16Ch buf[2] = { 0xffff, 0xffff };
    const Utf8Ch str[2] = { 0xdf, 0xbf };

    testAssertRequire(utf8ToUtf16(str, 2, buf, 2) == 1);
    testAssert(buf[0] == 0x07ff);
    testAssert(buf[1] == 0x0000);
}

void test_utf8ToUtf16SizeU0800(void) {
    const Utf8Ch str[3] = { 0xe0, 0xa0, 0x80 };
    testAssert(utf8ToUtf16(str, 3, NULL, 0) == 2);
}

void test_utf8ToUtf16U0800(void) {
    Utf16Ch buf[2] = { 0xffff, 0xffff };
    const Utf8Ch str[3] = { 0xe0, 0xa0, 0x80 };

    testAssertRequire(utf8ToUtf16(str, 3, buf, 2) == 1);
    testAssert(buf[0] == 0x0800);
    testAssert(buf[1] == 0x0000);
}

void test_utf8ToUtf16SizeUffff(void) {
    const Utf8Ch str[3] = { 0xef, 0xbf, 0xbf };
    testAssert(utf8ToUtf16(str, 3, NULL, 0) == 2);
}

void test_utf8ToUtf16Uffff(void) {
    Utf16Ch buf[2] = { 0xffff, 0xffff };
    const Utf8Ch str[3] = { 0xef, 0xbf, 0xbf };

    testAssertRequire(utf8ToUtf16(str, 3, buf, 2) == 1);
    testAssert(buf[0] == 0xffff);
    testAssert(buf[1] == 0x0000);
}

void test_utf8ToUtf16SizeU10000(void) {
    const Utf8Ch str[4] = { 0xf0, 0x90, 0x80, 0x80 };
    testAssert(utf8ToUtf16(str, 4, NULL, 0) == 3);
}

void test_utf8ToUtf16U10000(void) {
    Utf16Ch buf[3] = { 0xffff, 0xffff };
    const Utf8Ch str[4] = { 0xf0, 0x90, 0x80, 0x80 };

    testAssertRequire(utf8ToUtf16(str, 4, buf, 3) == 2);
    testAssert(buf[0] == ucdHighSurrogateFirst);
    testAssert(buf[1] == ucdLowSurrogateFirst);
    testAssert(buf[2] == 0x0000);
}

void test_utf8ToUtf16SizeU10ffff(void) {
    const Utf8Ch str[4] = { 0xf4, 0x8f, 0xbf, 0xbf};
    testAssert(utf8ToUtf16(str, 4, NULL, 0) == 3);
}

void test_utf8ToUtf16U10ffff(void) {
    Utf16Ch buf[3] = { 0xffff, 0xffff, 0xffff };
    const Utf8Ch str[4] = { 0xf4, 0x8f, 0xbf, 0xbf};

    testAssertRequire(utf8ToUtf16(str, 4, buf, 3) == 2);
    testAssert(buf[0] == ucdHighSurrogateLast);
    testAssert(buf[1] == ucdLowSurrogateLast);
    testAssert(buf[2] == 0x0000);
}

void test_utf8ToUtf16SizeMix(void) {
    const Utf8Ch str[20] = {
        0x00, // U+0000
        0x7f, // U+007F
        0xc2, 0x80, // U+0080
        0xdf, 0xbf, // U+07FF
        0xe0, 0xa0, 0x80, // U+0800
        0xef, 0xbf, 0xbf, // U+FFFF
        0xf0, 0x90, 0x80, 0x80, // U+10000
        0xf4, 0x8f, 0xbf, 0xbf // U+10FFFF
    };
    testAssert(utf8ToUtf16(str, 20, NULL, 0) == 11);
}

void test_utf8ToUtf16Mix(void) {
    Utf16Ch buf[11] = {
        0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
        0xffff, 0xffff
    };
    const Utf8Ch str[20] = {
        0x00, // U+0000
        0x7f, // U+007F
        0xc2, 0x80, // U+0080
        0xdf, 0xbf, // U+07FF
        0xe0, 0xa0, 0x80, // U+0800
        0xef, 0xbf, 0xbf, // U+FFFF
        0xf0, 0x90, 0x80, 0x80, // U+10000
        0xf4, 0x8f, 0xbf, 0xbf // U+10FFFF
    };
    testAssertRequire(utf8ToUtf16(str, 20, buf, 11) == 10);

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

void test_utf8ToUtf16SizeHighSurrogateFirst(void) {
    // ucdHighSurrogateFirst encoded in UTF8
    const Utf8Ch str[3] = { 0xed, 0xa0, 0x80 };
    testAssert(utf8ToUtf16(str, 3, NULL, 0) == 2);
}

void test_utf8ToUtf16HighSurrogateFirst(void) {
    Utf16Ch buf[2] = { 0xffff, 0xffff };
    // ucdHighSurrogateFirst encoded in UTF8
    const Utf8Ch str[3] = { 0xed, 0xa0, 0x80 };
    // invalid UTF8, U+FFFD (invalid character) is encoded
    testAssertRequire(utf8ToUtf16(str, 3, buf, 2) == 1);
    testAssert(buf[0] == 0xfffd);
    testAssert(buf[1] == 0x0000);
}

void test_utf8ToUtf16SizeHighSurrogateLast(void) {
    // ucdHighSurrogateLast encoded in UTF8
    const Utf8Ch str[3] = { 0xed, 0xaf, 0xbf };
    testAssert(utf8ToUtf16(str, 3, NULL, 0) == 2);
}

void test_utf8ToUtf16HighSurrogateLast(void) {
    Utf16Ch buf[2] = { 0xffff, 0xffff };
    // ucdHighSurrogateLast encoded in UTF8
    const Utf8Ch str[3] = { 0xed, 0xaf, 0xbf };
    // invalid UTF8, U+FFFD (invalid character) is encoded
    testAssertRequire(utf8ToUtf16(str, 3, buf, 2) == 1);
    testAssert(buf[0] == 0xfffd);
    testAssert(buf[1] == 0x0000);
}

void test_utf8ToUtf16SizeLowSurrogateFirst(void) {
    // ucdLowSurrogateFirst encoded in UTF8
    const Utf8Ch str[3] = { 0xed, 0xb0, 0x80 };
    testAssert(utf8ToUtf16(str, 3, NULL, 0) == 2);
}

void test_utf8ToUtf16LowSurrogateFirst(void) {
    Utf16Ch buf[2] = { 0xffff, 0xffff };
    // ucdLowSurrogateFirst encoded in UTF8
    const Utf8Ch str[3] = { 0xed, 0xb0, 0x80 };
    // invalid UTF8, U+FFFD (invalid character) is encoded
    testAssertRequire(utf8ToUtf16(str, 3, buf, 2) == 1);
    testAssert(buf[0] == 0xfffd);
    testAssert(buf[1] == 0x0000);
}

void test_utf8ToUtf16SizeLowSurrogateLast(void) {
    // ucdLowSurrogateLast encoded in UTF8
    const Utf8Ch str[3] = { 0xed, 0xbf, 0xbf };
    testAssert(utf8ToUtf16(str, 3, NULL, 0) == 2);
}

void test_utf8ToUtf16LowSurrogateLast(void) {
    Utf16Ch buf[2] = { 0xffff, 0xffff };
    // ucdLowSurrogateLast encoded in UTF8
    const Utf8Ch str[3] = { 0xed, 0xbf, 0xbf };
    // invalid UTF8, U+FFFD (invalid character) is encoded
    testAssertRequire(utf8ToUtf16(str, 3, buf, 2) == 1);
    testAssert(buf[0] == 0xfffd);
    testAssert(buf[1] == 0x0000);
}

void test_utf8ToUtf16SizeInvalidByte(void) {
    const Utf8Ch str[1] = { 0xff };
    testAssert(utf8ToUtf16(str, 1, NULL, 0) == 2);
}

void test_utf8ToUtf16InvalidByte(void) {
    Utf16Ch buf[2] = { 0xffff, 0xffff };
    const Utf8Ch str[1] = { 0xff };
    // invalid UTF8, U+FFFD (invalid character) is encoded
    testAssertRequire(utf8ToUtf16(str, 1, buf, 2) == 1);
    testAssert(buf[0] == 0xfffd);
    testAssert(buf[1] == 0x0000);
}

void test_utf8ToUtf16SizeIncomplete2Sequence(void) {
    const Utf8Ch str[1] = { 0xc2 };
    testAssert(utf8ToUtf16(str, 1, NULL, 0) == 2);
}

void test_utf8ToUtf16Incomplete2Sequence(void) {
    Utf16Ch buf[2] = { 0xffff, 0xffff };
    const Utf8Ch str[1] = { 0xc2 };
    // invalid UTF8, U+FFFD (invalid character) is encoded
    testAssertRequire(utf8ToUtf16(str, 1, buf, 2) == 1);
    testAssert(buf[0] == 0xfffd);
    testAssert(buf[1] == 0x0000);
}

void test_utf8ToUtf16SizeIncorrect2Sequence(void) {
    const Utf8Ch str[2] = { 0xc2, 0x01 };
    testAssert(utf8ToUtf16(str, 2, NULL, 0) == 3);
}

void test_utf8ToUtf16Incorrect2Sequence(void) {
    Utf16Ch buf[3] = { 0xffff, 0xffff, 0xffff };
    const Utf8Ch str[2] = { 0xc2, 0x01 };
    // invalid UTF8, U+FFFD (invalid character) is encoded
    testAssertRequire(utf8ToUtf16(str, 2, buf, 3) == 2);
    testAssert(buf[0] == 0xfffd);
    testAssert(buf[1] == 0x0001);
    testAssert(buf[2] == 0x0000);
}

void test_utf8ToUtf16SizeIncomplete3Sequence(void) {
    const Utf8Ch str[2] = { 0xe0, 0x80 };
    testAssert(utf8ToUtf16(str, 2, NULL, 0) == 3);
}

void test_utf8ToUtf16Incomplete3Sequence(void) {
    Utf16Ch buf[3] = { 0xffff, 0xffff, 0xffff };
    const Utf8Ch str[2] = { 0xe0, 0x80 };
    // invalid UTF8, U+FFFD (invalid character) is encoded
    testAssertRequire(utf8ToUtf16(str, 2, buf, 3) == 2);
    testAssert(buf[0] == 0xfffd);
    testAssert(buf[1] == 0xfffd);
    testAssert(buf[2] == 0x0000);
}

void test_utf8ToUtf16SizeIncorrect3Sequence(void) {
    const Utf8Ch str[3] = { 0xe0, 0x80, 0x01 };
    testAssert(utf8ToUtf16(str, 3, NULL, 0) == 4);
}

void test_utf8ToUtf16Incorrect3Sequence(void) {
    Utf16Ch buf[4] = { 0xffff, 0xffff, 0xffff, 0xffff };
    const Utf8Ch str[3] = { 0xe0, 0x80, 0x01 };
    // invalid UTF8, U+FFFD (invalid character) is encoded
    testAssertRequire(utf8ToUtf16(str, 3, buf, 4) == 3);
    testAssert(buf[0] == 0xfffd);
    testAssert(buf[1] == 0xfffd);
    testAssert(buf[2] == 0x0001);
    testAssert(buf[3] == 0x0000);
}

void test_utf8ToUtf16SizeInvalid3Sequence(void) {
    const Utf8Ch str[3] = { 0xe0, 0x80, 0x80 };
    testAssert(utf8ToUtf16(str, 3, NULL, 0) == 2);
}

void test_utf8ToUtf16Invalid3Sequence(void) {
    Utf16Ch buf[2] = { 0xffff, 0xffff };
    const Utf8Ch str[3] = { 0xe0, 0x80, 0x80 };
    // invalid UTF8, U+FFFD (invalid character) is encoded
    testAssertRequire(utf8ToUtf16(str, 3, buf, 2) == 1);
    testAssert(buf[0] == 0xfffd);
    testAssert(buf[1] == 0x0000);
}

void test_utf8ToUtf16SizeIncomplete4Sequence(void) {
    const Utf8Ch str[3] = { 0xf0, 0x80, 0x80 };
    testAssert(utf8ToUtf16(str, 3, NULL, 0) == 4);
}

void test_utf8ToUtf16Incomplete4Sequence(void) {
    Utf16Ch buf[4] = { 0xffff, 0xffff, 0xffff, 0xffff };
    const Utf8Ch str[3] = { 0xf0, 0x80, 0x80 };
    // invalid UTF8, U+FFFD (invalid character) is encoded
    testAssertRequire(utf8ToUtf16(str, 3, buf, 4) == 3);
    testAssert(buf[0] == 0xfffd);
    testAssert(buf[1] == 0xfffd);
    testAssert(buf[2] == 0xfffd);
    testAssert(buf[3] == 0x0000);
}

void test_utf8ToUtf16SizeIncorrect4Sequence(void) {
    const Utf8Ch str[4] = { 0xf0, 0x80, 0x80, 0x01 };
    testAssert(utf8ToUtf16(str, 4, NULL, 0) == 5);
}

void test_utf8ToUtf16Incorrect4Sequence(void) {
    Utf16Ch buf[5] = { 0xffff, 0xffff, 0xffff, 0xffff, 0xffff };
    const Utf8Ch str[4] = { 0xf0, 0x80, 0x80, 0x01 };
    // invalid UTF8, U+FFFD (invalid character) is encoded
    testAssertRequire(utf8ToUtf16(str, 4, buf, 5) == 4);
    testAssert(buf[0] == 0xfffd);
    testAssert(buf[1] == 0xfffd);
    testAssert(buf[2] == 0xfffd);
    testAssert(buf[3] == 0x0001);
    testAssert(buf[4] == 0x0000);
}

void test_utf8ToUtf16SizeInvalid4Sequence(void) {
    const Utf8Ch str[4] = { 0xf0, 0x80, 0x80, 0x80 };
    testAssert(utf8ToUtf16(str, 4, NULL, 0) == 2);
}

void test_utf8ToUtf16Invalid4Sequence(void) {
    Utf16Ch buf[2] = { 0xffff, 0xffff };
    const Utf8Ch str[4] = { 0xf0, 0x80, 0x80, 0x80 };
    // invalid UTF8, U+FFFD (invalid character) is encoded
    testAssertRequire(utf8ToUtf16(str, 4, buf, 2) == 1);
    testAssert(buf[0] == 0xfffd);
    testAssert(buf[1] == 0x0000);
}

testList(
    testMake(test_utf8ToUtf16SizeU0000),
    testMake(test_utf8ToUtf16U0000),
    testMake(test_utf8ToUtf16SizeU007f),
    testMake(test_utf8ToUtf16U007f),
    testMake(test_utf8ToUtf16SizeU0000U007f),
    testMake(test_utf8ToUtf16U0000U007f),
    testMake(test_utf8ToUtf16SizeU0080),
    testMake(test_utf8ToUtf16U0080),
    testMake(test_utf8ToUtf16SizeU07ff),
    testMake(test_utf8ToUtf16U07ff),
    testMake(test_utf8ToUtf16SizeU0800),
    testMake(test_utf8ToUtf16U0800),
    testMake(test_utf8ToUtf16SizeUffff),
    testMake(test_utf8ToUtf16Uffff),
    testMake(test_utf8ToUtf16SizeU10000),
    testMake(test_utf8ToUtf16U10000),
    testMake(test_utf8ToUtf16SizeMix),
    testMake(test_utf8ToUtf16Mix),
    testMake(test_utf8ToUtf16SizeHighSurrogateFirst),
    testMake(test_utf8ToUtf16HighSurrogateFirst),
    testMake(test_utf8ToUtf16SizeHighSurrogateLast),
    testMake(test_utf8ToUtf16HighSurrogateFirst),
    testMake(test_utf8ToUtf16SizeLowSurrogateFirst),
    testMake(test_utf8ToUtf16LowSurrogateFirst),
    testMake(test_utf8ToUtf16SizeLowSurrogateLast),
    testMake(test_utf8ToUtf16LowSurrogateLast),
    testMake(test_utf8ToUtf16SizeInvalidByte),
    testMake(test_utf8ToUtf16InvalidByte),
    testMake(test_utf8ToUtf16SizeIncomplete2Sequence),
    testMake(test_utf8ToUtf16Incomplete2Sequence),
    testMake(test_utf8ToUtf16SizeIncorrect2Sequence),
    testMake(test_utf8ToUtf16Incorrect2Sequence),
    testMake(test_utf8ToUtf16SizeIncomplete3Sequence),
    testMake(test_utf8ToUtf16Incomplete3Sequence),
    testMake(test_utf8ToUtf16SizeIncorrect3Sequence),
    testMake(test_utf8ToUtf16Incorrect3Sequence),
    testMake(test_utf8ToUtf16SizeInvalid3Sequence),
    testMake(test_utf8ToUtf16Invalid3Sequence),
    testMake(test_utf8ToUtf16SizeIncomplete4Sequence),
    testMake(test_utf8ToUtf16Incomplete4Sequence),
    testMake(test_utf8ToUtf16SizeIncorrect4Sequence),
    testMake(test_utf8ToUtf16Incorrect4Sequence),
    testMake(test_utf8ToUtf16SizeInvalid4Sequence),
    testMake(test_utf8ToUtf16Invalid4Sequence),
);
