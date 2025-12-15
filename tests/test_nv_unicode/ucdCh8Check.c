#include "nv_test.h"
#include "nv_unicode.h"

void test_ucdCh8CheckU0000(void) {
    const UcdCh8 str[1] = { 0x00 };
    testAssert(ucdCh8Check(str, 1));
}

void test_ucdCh8CheckU007f(void) {
    const UcdCh8 str[1] = { 0x7f };
    testAssert(ucdCh8Check(str, 1));
}

void test_ucdCh8CheckU0000U007f(void) {
    const UcdCh8 str[2] = { 0x00, 0x7f };
    testAssert(ucdCh8Check(str, 2));
}

void test_ucdCh8CheckU0080(void) {
    const UcdCh8 str[2] = { 0xc2, 0x80 };
    testAssert(ucdCh8Check(str, 2));
}

void test_ucdCh8CheckU07ff(void) {
    const UcdCh8 str[2] = { 0xdf, 0xbf };
    testAssert(ucdCh8Check(str, 2));
}

void test_ucdCh8CheckU0800(void) {
    const UcdCh8 str[3] = { 0xe0, 0xa0, 0x80 };
    testAssert(ucdCh8Check(str, 3));
}

void test_ucdCh8CheckUffff(void) {
    const UcdCh8 str[3] = { 0xef, 0xbf, 0xbf };
    testAssert(ucdCh8Check(str, 3));
}

void test_ucdCh8CheckU10000(void) {
    const UcdCh8 str[4] = { 0xf0, 0x90, 0x80, 0x80 };
    testAssert(ucdCh8Check(str, 4));
}

void test_ucdCh8CheckU10ffff(void) {
    const UcdCh8 str[4] = { 0xf4, 0x8f, 0xbf, 0xbf};
    testAssert(ucdCh8Check(str, 4));
}

void test_ucdCh8CheckMix(void) {
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
    testAssert(ucdCh8Check(str, 20));
}

void test_ucdCh8CheckHighSurrogateFirst(void) {
    // ucdHighSurrogateFirst encoded in UTF8
    const UcdCh8 str[3] = { 0xed, 0xa0, 0x80 };
    testAssert(!ucdCh8Check(str, 3));
}

void test_ucdCh8CheckHighSurrogateLast(void) {
    // ucdHighSurrogateLast encoded in UTF8
    const UcdCh8 str[3] = { 0xed, 0xaf, 0xbf };
    testAssert(!ucdCh8Check(str, 3));
}

void test_ucdCh8CheckLowSurrogateFirst(void) {
    // ucdLowSurrogateFirst encoded in UTF8
    const UcdCh8 str[3] = { 0xed, 0xb0, 0x80 };
    testAssert(!ucdCh8Check(str, 3));
}

void test_ucdCh8CheckLowSurrogateLast(void) {
    // ucdLowSurrogateLast encoded in UTF8
    const UcdCh8 str[3] = { 0xed, 0xbf, 0xbf };
    testAssert(!ucdCh8Check(str, 3));
}

void test_ucdCh8CheckInvalidByte(void) {
    const UcdCh8 str[1] = { 0xff };
    testAssert(!ucdCh8Check(str, 1));
}

void test_ucdCh8CheckIncomplete2Sequence(void) {
    const UcdCh8 str[1] = { 0xc2 };
    testAssert(!ucdCh8Check(str, 1));
}

void test_ucdCh8CheckIncorrect2Sequence(void) {
    const UcdCh8 str[2] = { 0xc2, 0x01 };
    testAssert(!ucdCh8Check(str, 2));
}

void test_ucdCh8CheckIncomplete3Sequence(void) {
    const UcdCh8 str[2] = { 0xe0, 0x80 };
    testAssert(!ucdCh8Check(str, 2));
}

void test_ucdCh8CheckIncorrect3Sequence(void) {
    const UcdCh8 str[3] = { 0xe0, 0x80, 0x01 };
    testAssert(!ucdCh8Check(str, 3));
}

void test_ucdCh8CheckInvalid3Sequence(void) {
    const UcdCh8 str[3] = { 0xe0, 0x80, 0x80 };
    testAssert(!ucdCh8Check(str, 3));
}

void test_ucdCh8CheckIncomplete4Sequence(void) {
    const UcdCh8 str[3] = { 0xf0, 0x80, 0x80 };
    testAssert(!ucdCh8Check(str, 3));
}

void test_ucdCh8CheckIncorrect4Sequence(void) {
    const UcdCh8 str[4] = { 0xf0, 0x80, 0x80, 0x01 };
    testAssert(!ucdCh8Check(str, 4));
}

void test_ucdCh8CheckInvalid4Sequence(void) {
    const UcdCh8 str[4] = { 0xf0, 0x80, 0x80, 0x80 };
    testAssert(!ucdCh8Check(str, 4));
}

testList(
    testMake(test_ucdCh8CheckU0000),
    testMake(test_ucdCh8CheckU007f),
    testMake(test_ucdCh8CheckU0000U007f),
    testMake(test_ucdCh8CheckU0080),
    testMake(test_ucdCh8CheckU07ff),
    testMake(test_ucdCh8CheckU0800),
    testMake(test_ucdCh8CheckUffff),
    testMake(test_ucdCh8CheckU10000),
    testMake(test_ucdCh8CheckMix),
    testMake(test_ucdCh8CheckHighSurrogateFirst),
    testMake(test_ucdCh8CheckHighSurrogateLast),
    testMake(test_ucdCh8CheckLowSurrogateFirst),
    testMake(test_ucdCh8CheckLowSurrogateLast),
    testMake(test_ucdCh8CheckInvalidByte),
    testMake(test_ucdCh8CheckIncomplete2Sequence),
    testMake(test_ucdCh8CheckIncorrect2Sequence),
    testMake(test_ucdCh8CheckIncomplete3Sequence),
    testMake(test_ucdCh8CheckIncorrect3Sequence),
    testMake(test_ucdCh8CheckInvalid3Sequence),
    testMake(test_ucdCh8CheckIncomplete4Sequence),
    testMake(test_ucdCh8CheckIncorrect4Sequence),
    testMake(test_ucdCh8CheckInvalid4Sequence),
);
