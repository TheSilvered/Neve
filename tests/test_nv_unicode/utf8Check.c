#include "nv_test.h"
#include "unicode/nv_utf.h"

void test_utf8CheckU0000(void) {
    const Utf8Ch str[1] = { 0x00 };
    testAssert(utf8Check(str, 1));
}

void test_utf8CheckU007f(void) {
    const Utf8Ch str[1] = { 0x7f };
    testAssert(utf8Check(str, 1));
}

void test_utf8CheckU0000U007f(void) {
    const Utf8Ch str[2] = { 0x00, 0x7f };
    testAssert(utf8Check(str, 2));
}

void test_utf8CheckU0080(void) {
    const Utf8Ch str[2] = { 0xc2, 0x80 };
    testAssert(utf8Check(str, 2));
}

void test_utf8CheckU07ff(void) {
    const Utf8Ch str[2] = { 0xdf, 0xbf };
    testAssert(utf8Check(str, 2));
}

void test_utf8CheckU0800(void) {
    const Utf8Ch str[3] = { 0xe0, 0xa0, 0x80 };
    testAssert(utf8Check(str, 3));
}

void test_utf8CheckUffff(void) {
    const Utf8Ch str[3] = { 0xef, 0xbf, 0xbf };
    testAssert(utf8Check(str, 3));
}

void test_utf8CheckU10000(void) {
    const Utf8Ch str[4] = { 0xf0, 0x90, 0x80, 0x80 };
    testAssert(utf8Check(str, 4));
}

void test_utf8CheckU10ffff(void) {
    const Utf8Ch str[4] = { 0xf4, 0x8f, 0xbf, 0xbf};
    testAssert(utf8Check(str, 4));
}

void test_utf8CheckMix(void) {
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
    testAssert(utf8Check(str, 20));
}

void test_utf8CheckHighSurrogateFirst(void) {
    // ucdHighSurrogateFirst encoded in UTF8
    const Utf8Ch str[3] = { 0xed, 0xa0, 0x80 };
    testAssert(!utf8Check(str, 3));
}

void test_utf8CheckHighSurrogateLast(void) {
    // ucdHighSurrogateLast encoded in UTF8
    const Utf8Ch str[3] = { 0xed, 0xaf, 0xbf };
    testAssert(!utf8Check(str, 3));
}

void test_utf8CheckLowSurrogateFirst(void) {
    // ucdLowSurrogateFirst encoded in UTF8
    const Utf8Ch str[3] = { 0xed, 0xb0, 0x80 };
    testAssert(!utf8Check(str, 3));
}

void test_utf8CheckLowSurrogateLast(void) {
    // ucdLowSurrogateLast encoded in UTF8
    const Utf8Ch str[3] = { 0xed, 0xbf, 0xbf };
    testAssert(!utf8Check(str, 3));
}

void test_utf8CheckInvalidByte(void) {
    const Utf8Ch str[1] = { 0xff };
    testAssert(!utf8Check(str, 1));
}

void test_utf8CheckIncomplete2Sequence(void) {
    const Utf8Ch str[1] = { 0xc2 };
    testAssert(!utf8Check(str, 1));
}

void test_utf8CheckIncorrect2Sequence(void) {
    const Utf8Ch str[2] = { 0xc2, 0x01 };
    testAssert(!utf8Check(str, 2));
}

void test_utf8CheckIncomplete3Sequence(void) {
    const Utf8Ch str[2] = { 0xe0, 0x80 };
    testAssert(!utf8Check(str, 2));
}

void test_utf8CheckIncorrect3Sequence(void) {
    const Utf8Ch str[3] = { 0xe0, 0x80, 0x01 };
    testAssert(!utf8Check(str, 3));
}

void test_utf8CheckInvalid3Sequence(void) {
    const Utf8Ch str[3] = { 0xe0, 0x80, 0x80 };
    testAssert(!utf8Check(str, 3));
}

void test_utf8CheckIncomplete4Sequence(void) {
    const Utf8Ch str[3] = { 0xf0, 0x80, 0x80 };
    testAssert(!utf8Check(str, 3));
}

void test_utf8CheckIncorrect4Sequence(void) {
    const Utf8Ch str[4] = { 0xf0, 0x80, 0x80, 0x01 };
    testAssert(!utf8Check(str, 4));
}

void test_utf8CheckInvalid4Sequence(void) {
    const Utf8Ch str[4] = { 0xf0, 0x80, 0x80, 0x80 };
    testAssert(!utf8Check(str, 4));
}

testList(
    testMake(test_utf8CheckU0000),
    testMake(test_utf8CheckU007f),
    testMake(test_utf8CheckU0000U007f),
    testMake(test_utf8CheckU0080),
    testMake(test_utf8CheckU07ff),
    testMake(test_utf8CheckU0800),
    testMake(test_utf8CheckUffff),
    testMake(test_utf8CheckU10000),
    testMake(test_utf8CheckMix),
    testMake(test_utf8CheckHighSurrogateFirst),
    testMake(test_utf8CheckHighSurrogateLast),
    testMake(test_utf8CheckLowSurrogateFirst),
    testMake(test_utf8CheckLowSurrogateLast),
    testMake(test_utf8CheckInvalidByte),
    testMake(test_utf8CheckIncomplete2Sequence),
    testMake(test_utf8CheckIncorrect2Sequence),
    testMake(test_utf8CheckIncomplete3Sequence),
    testMake(test_utf8CheckIncorrect3Sequence),
    testMake(test_utf8CheckInvalid3Sequence),
    testMake(test_utf8CheckIncomplete4Sequence),
    testMake(test_utf8CheckIncorrect4Sequence),
    testMake(test_utf8CheckInvalid4Sequence),
);
