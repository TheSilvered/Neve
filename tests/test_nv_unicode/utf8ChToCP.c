#include "nv_test.h"
#include "unicode/nv_utf.h"

void test_utf8ChToCP1Byte(void) {
    const Utf8Ch bytes0[1] = { 0x00 };
    const Utf8Ch bytes1[1] = { 0x7f };

    testAssert(utf8ChToCP(bytes0) == 0x0000);
    testAssert(utf8ChToCP(bytes1) == 0x007f);
}

void test_utf8ChToCP2Bytes(void) {
    const Utf8Ch bytes0[2] = { 0xc2, 0x80 };
    const Utf8Ch bytes1[2] = { 0xdf, 0xbf };

    testAssert(utf8ChToCP(bytes0) == 0x0080);
    testAssert(utf8ChToCP(bytes1) == 0x07ff);
}

void test_utf8ChToCP3Bytes(void) {
    const Utf8Ch bytes0[3] = { 0xe0, 0xa0, 0x80 };
    const Utf8Ch bytes1[3] = { 0xef, 0xbf, 0xbf };

    testAssert(utf8ChToCP(bytes0) == 0x0800);
    testAssert(utf8ChToCP(bytes1) == 0xffff);
}

void test_utf8ChToCP4Bytes(void) {
    const Utf8Ch bytes0[4] = { 0xf0, 0x90, 0x80, 0x80 };
    const Utf8Ch bytes1[4] = { 0xf4, 0x8f, 0xbf, 0xbf };

    testAssert(utf8ChToCP(bytes0) == 0x10000);
    testAssert(utf8ChToCP(bytes1) == 0x10ffff);
}

testList(
    testMake(test_utf8ChToCP1Byte),
    testMake(test_utf8ChToCP2Bytes),
    testMake(test_utf8ChToCP3Bytes),
    testMake(test_utf8ChToCP4Bytes)
)
