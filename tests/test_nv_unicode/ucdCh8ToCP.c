#include "nv_test.h"
#include "nv_unicode.h"

void test_ucdCh8ToCP1Byte(void) {
    const UcdCh8 bytes0[1] = { 0x00 };
    const UcdCh8 bytes1[1] = { 0x7f };

    testAssert(ucdCh8ToCP(bytes0) == 0x0000);
    testAssert(ucdCh8ToCP(bytes1) == 0x007f);
}

void test_ucdCh8ToCP2Bytes(void) {
    const UcdCh8 bytes0[2] = { 0xc2, 0x80 };
    const UcdCh8 bytes1[2] = { 0xdf, 0xbf };

    testAssert(ucdCh8ToCP(bytes0) == 0x0080);
    testAssert(ucdCh8ToCP(bytes1) == 0x07ff);
}

void test_ucdCh8ToCP3Bytes(void) {
    const UcdCh8 bytes0[3] = { 0xe0, 0xa0, 0x80 };
    const UcdCh8 bytes1[3] = { 0xef, 0xbf, 0xbf };

    testAssert(ucdCh8ToCP(bytes0) == 0x0800);
    testAssert(ucdCh8ToCP(bytes1) == 0xffff);
}

void test_ucdCh8ToCP4Bytes(void) {
    const UcdCh8 bytes0[4] = { 0xf0, 0x90, 0x80, 0x80 };
    const UcdCh8 bytes1[4] = { 0xf4, 0x8f, 0xbf, 0xbf };

    testAssert(ucdCh8ToCP(bytes0) == 0x10000);
    testAssert(ucdCh8ToCP(bytes1) == 0x10ffff);
}

testList(
    testMake(test_ucdCh8ToCP1Byte),
    testMake(test_ucdCh8ToCP2Bytes),
    testMake(test_ucdCh8ToCP3Bytes),
    testMake(test_ucdCh8ToCP4Bytes)
)
