#include "nv_test.h"
#include "unicode/nv_utf.h"

void test_utf16ToUtf8SizeU0000(void) {
    const Utf16Ch str[1] = { 0x0000 };
    testAssert(utf16ToUtf8(str, 1, NULL, 0) == 2);
}

void test_utf16ToUtf8U0000(void) {
    Utf8Ch buf[2] = { 0xff, 0xff };
    const Utf16Ch str[1] = { 0x0000 };

    testAssertRequire(utf16ToUtf8(str, 1, buf, 2) == 1);
    testAssert(buf[0] == 0x00);
    testAssert(buf[1] == 0x00);
}

void test_utf16ToUtf8SizeU007f(void) {
    const Utf16Ch str[1] = { 0x007f };
    testAssert(utf16ToUtf8(str, 1, NULL, 0) == 2);
}

void test_utf16ToUtf8U007f(void) {
    Utf8Ch buf[2] = { 0xff, 0xff };
    const Utf16Ch str[1] = { 0x007f };

    testAssertRequire(utf16ToUtf8(str, 1, buf, 2) == 1);
    testAssert(buf[0] == 0x7f);
    testAssert(buf[1] == 0x00);
}

void test_utf16ToUtf8SizeU0000U007f(void) {
    const Utf16Ch str[2] = { 0x0000, 0x007f };
    testAssert(utf16ToUtf8(str, 2, NULL, 0) == 3);
}

void test_utf16ToUtf8U0000U007f(void) {
    Utf8Ch buf[3] = { 0xff, 0xff, 0xff };
    const Utf16Ch str[2] = { 0x0000, 0x007f };

    testAssertRequire(utf16ToUtf8(str, 2, buf, 3) == 2);
    testAssert(buf[0] == 0x00);
    testAssert(buf[1] == 0x7f);
    testAssert(buf[2] == 0x00);
}

void test_utf16ToUtf8SizeU0080(void) {
    const Utf16Ch str[1] = { 0x0080 };
    testAssert(utf16ToUtf8(str, 1, NULL, 0) == 3);
}

void test_utf16ToUtf8U0080(void) {
    Utf8Ch buf[3] = { 0xff, 0xff, 0xff };
    const Utf16Ch str[1] = { 0x0080 };

    testAssertRequire(utf16ToUtf8(str, 1, buf, 3) == 2);
    testAssert(buf[0] == 0xc2);
    testAssert(buf[1] == 0x80);
    testAssert(buf[2] == 0x00);
}

void test_utf16ToUtf8SizeU07ff(void) {
    const Utf16Ch str[1] = { 0x07ff };
    testAssert(utf16ToUtf8(str, 1, NULL, 0) == 3);
}

void test_utf16ToUtf8U07ff(void) {
    Utf8Ch buf[3] = { 0xff, 0xff, 0xff };
    const Utf16Ch str[1] = { 0x07ff };

    testAssertRequire(utf16ToUtf8(str, 1, buf, 3) == 2);
    testAssert(buf[0] == 0xdf);
    testAssert(buf[1] == 0xbf);
    testAssert(buf[2] == 0x00);
}

void test_utf16ToUtf8SizeU0800(void) {
    const Utf16Ch str[1] = { 0x0800 };
    testAssert(utf16ToUtf8(str, 1, NULL, 0) == 4);
}

void test_utf16ToUtf8U0800(void) {
    Utf8Ch buf[4] = { 0xff, 0xff, 0xff, 0xff };
    const Utf16Ch str[1] = { 0x0800 };

    testAssertRequire(utf16ToUtf8(str, 1, buf, 4) == 3);
    testAssert(buf[0] == 0xe0);
    testAssert(buf[1] == 0xa0);
    testAssert(buf[2] == 0x80);
    testAssert(buf[3] == 0x00);
}

void test_utf16ToUtf8SizeUffff(void) {
    const Utf16Ch str[1] = { 0xffff };
    testAssert(utf16ToUtf8(str, 1, NULL, 0) == 4);
}

void test_utf16ToUtf8Uffff(void) {
    Utf8Ch buf[4] = { 0xff, 0xff, 0xff, 0xff };
    const Utf16Ch str[1] = { 0xffff };

    testAssertRequire(utf16ToUtf8(str, 1, buf, 4) == 3);
    testAssert(buf[0] == 0xef);
    testAssert(buf[1] == 0xbf);
    testAssert(buf[2] == 0xbf);
    testAssert(buf[3] == 0x00);
}

void test_utf16ToUtf8SizeU10000(void) {
    const Utf16Ch str[2] = {
        ucdHighSurrogateFirst,
        ucdLowSurrogateFirst
    };
    testAssert(utf16ToUtf8(str, 2, NULL, 0) == 5);
}

void test_utf16ToUtf8U10000(void) {
    Utf8Ch buf[5] = { 0xff, 0xff, 0xff, 0xff, 0xff };
    const Utf16Ch str[2] = {
        ucdHighSurrogateFirst,
        ucdLowSurrogateFirst
    };

    testAssertRequire(utf16ToUtf8(str, 2, buf, 5) == 4);
    testAssert(buf[0] == 0xf0);
    testAssert(buf[1] == 0x90);
    testAssert(buf[2] == 0x80);
    testAssert(buf[3] == 0x80);
    testAssert(buf[4] == 0x00);
}

void test_utf16ToUtf8SizeU10ffff(void) {
    const Utf16Ch str[2] = {
        ucdHighSurrogateLast,
        ucdLowSurrogateLast
    };
    testAssert(utf16ToUtf8(str, 2, NULL, 0) == 5);
}

void test_utf16ToUtf8U10ffff(void) {
    Utf8Ch buf[5] = { 0xff, 0xff, 0xff, 0xff, 0xff };
    const Utf16Ch str[2] = {
        ucdHighSurrogateLast,
        ucdLowSurrogateLast
    };

    testAssertRequire(utf16ToUtf8(str, 2, buf, 5) == 4);
    testAssert(buf[0] == 0xf4);
    testAssert(buf[1] == 0x8f);
    testAssert(buf[2] == 0xbf);
    testAssert(buf[3] == 0xbf);
    testAssert(buf[4] == 0x00);
}

void test_utf16ToUtf8SizeMix(void) {
    const Utf16Ch str[10] = {
        0x0000,
        0x007f,
        0x0080,
        0x07ff,
        0x0800,
        0xffff,
        ucdHighSurrogateFirst, ucdLowSurrogateFirst,
        ucdHighSurrogateLast, ucdLowSurrogateLast
    };
    testAssert(utf16ToUtf8(str, 10, NULL, 0) == 21);
}

void test_utf16ToUtf8Mix(void) {
    Utf8Ch buf[21] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    const Utf16Ch str[10] = {
        0x0000,
        0x007f,
        0x0080,
        0x07ff,
        0x0800,
        0xffff,
        ucdHighSurrogateFirst, ucdLowSurrogateFirst,
        ucdHighSurrogateLast, ucdLowSurrogateLast
    };
    testAssertRequire(utf16ToUtf8(str, 10, buf, 21) == 20);
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

void test_utf16ToUtf8SizeHighSurrogateFirst(void) {
    const Utf16Ch str[1] = { ucdHighSurrogateFirst };
    testAssert(utf16ToUtf8(str, 1, NULL, 0) == 4);
}

void test_utf16ToUtf8HighSurrogateFirst(void) {
    Utf8Ch buf[4] = { 0xff, 0xff, 0xff, 0xff };
    const Utf16Ch str[1] = { ucdHighSurrogateFirst };
    // invalid UTF16, U+FFFD (invalid character) is encoded
    testAssertRequire(utf16ToUtf8(str, 1, buf, 4) == 3);
    testAssert(buf[0] == 0xef);
    testAssert(buf[1] == 0xbf);
    testAssert(buf[2] == 0xbd);
    testAssert(buf[3] == 0x00);
}

void test_utf16ToUtf8SizeHighSurrogateLast(void) {
    const Utf16Ch str[1] = { ucdHighSurrogateLast };
    testAssert(utf16ToUtf8(str, 1, NULL, 0) == 4);
}

void test_utf16ToUtf8HighSurrogateLast(void) {
    Utf8Ch buf[4] = { 0xff, 0xff, 0xff, 0xff };
    const Utf16Ch str[1] = { ucdHighSurrogateLast };
    // invalid UTF16, U+FFFD (invalid character) is encoded
    testAssertRequire(utf16ToUtf8(str, 1, buf, 4) == 3);
    testAssert(buf[0] == 0xef);
    testAssert(buf[1] == 0xbf);
    testAssert(buf[2] == 0xbd);
    testAssert(buf[3] == 0x00);
}

void test_utf16ToUtf8SizeLowSurrogateFirst(void) {
    const Utf16Ch str[1] = { ucdLowSurrogateFirst };
    testAssert(utf16ToUtf8(str, 1, NULL, 0) == 4);
}

void test_utf16ToUtf8LowSurrogateFirst(void) {
    Utf8Ch buf[4] = { 0xff, 0xff, 0xff, 0xff };
    const Utf16Ch str[1] = { ucdLowSurrogateFirst };
    // invalid UTF16, U+FFFD (invalid character) is encoded
    testAssertRequire(utf16ToUtf8(str, 1, buf, 4) == 3);
    testAssert(buf[0] == 0xef);
    testAssert(buf[1] == 0xbf);
    testAssert(buf[2] == 0xbd);
    testAssert(buf[3] == 0x00);
}

void test_utf16ToUtf8SizeLowSurrogateLast(void) {
    const Utf16Ch str[1] = { ucdLowSurrogateLast };
    testAssert(utf16ToUtf8(str, 1, NULL, 0) == 4);
}

void test_utf16ToUtf8LowSurrogateLast(void) {
    Utf8Ch buf[4] = { 0xff, 0xff, 0xff, 0xff };
    const Utf16Ch str[1] = { ucdLowSurrogateLast };
    // invalid UTF16, U+FFFD (invalid character) is encoded
    testAssertRequire(utf16ToUtf8(str, 1, buf, 4) == 3);
    testAssert(buf[0] == 0xef);
    testAssert(buf[1] == 0xbf);
    testAssert(buf[2] == 0xbd);
    testAssert(buf[3] == 0x00);
}

void test_utf16ToUtf8SizeIncompleteSurrogate(void) {
    const Utf16Ch str[2] = { ucdHighSurrogateFirst, 1 };
    testAssert(utf16ToUtf8(str, 2, NULL, 0) == 5);
}

void test_utf16ToUtf8IncompleteSurrogate(void) {
    Utf8Ch buf[5] = { 0xff, 0xff, 0xff, 0xff };
    const Utf16Ch str[2] = { ucdHighSurrogateFirst, 1 };
    // invalid UTF16, U+FFFD (invalid character) is encoded
    testAssertRequire(utf16ToUtf8(str, 2, buf, 5) == 4);
    testAssert(buf[0] == 0xef);
    testAssert(buf[1] == 0xbf);
    testAssert(buf[2] == 0xbd);
    testAssert(buf[3] == 0x01);
    testAssert(buf[4] == 0x00);
}

testList(
    testMake(test_utf16ToUtf8SizeU0000),
    testMake(test_utf16ToUtf8U0000),
    testMake(test_utf16ToUtf8SizeU007f),
    testMake(test_utf16ToUtf8U007f),
    testMake(test_utf16ToUtf8SizeU0000U007f),
    testMake(test_utf16ToUtf8U0000U007f),
    testMake(test_utf16ToUtf8SizeU0080),
    testMake(test_utf16ToUtf8U0080),
    testMake(test_utf16ToUtf8SizeU07ff),
    testMake(test_utf16ToUtf8U07ff),
    testMake(test_utf16ToUtf8SizeU0800),
    testMake(test_utf16ToUtf8U0800),
    testMake(test_utf16ToUtf8SizeUffff),
    testMake(test_utf16ToUtf8Uffff),
    testMake(test_utf16ToUtf8SizeU10000),
    testMake(test_utf16ToUtf8U10000),
    testMake(test_utf16ToUtf8SizeU10ffff),
    testMake(test_utf16ToUtf8U10ffff),
    testMake(test_utf16ToUtf8SizeMix),
    testMake(test_utf16ToUtf8Mix),
    testMake(test_utf16ToUtf8SizeHighSurrogateFirst),
    testMake(test_utf16ToUtf8HighSurrogateFirst),
    testMake(test_utf16ToUtf8SizeHighSurrogateLast),
    testMake(test_utf16ToUtf8HighSurrogateLast),
    testMake(test_utf16ToUtf8SizeLowSurrogateFirst),
    testMake(test_utf16ToUtf8LowSurrogateFirst),
    testMake(test_utf16ToUtf8SizeLowSurrogateLast),
    testMake(test_utf16ToUtf8LowSurrogateLast),
    testMake(test_utf16ToUtf8SizeIncompleteSurrogate),
    testMake(test_utf16ToUtf8IncompleteSurrogate)
)
