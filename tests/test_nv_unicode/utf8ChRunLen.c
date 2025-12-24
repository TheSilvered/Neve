#include "nv_test.h"
#include "unicode/nv_utf.h"

void test_utf8ChRunLen1Byte(void) {
    testAssert(utf8ChRunLen(0x0) == 1);
    testAssert(utf8ChRunLen('a') == 1);
}

void test_utf8ChRunLen2Bytes(void) {
    testAssert(utf8ChRunLen(0xc2) == 2);
    testAssert(utf8ChRunLen(0xdf) == 2);
}

void test_utf8ChRunLen3Bytes(void) {
    testAssert(utf8ChRunLen(0xe0) == 3);
    testAssert(utf8ChRunLen(0xef) == 3);
}

void test_utf8ChRunLen4Bytes(void) {
    testAssert(utf8ChRunLen(0xf0) == 4);
    testAssert(utf8ChRunLen(0xf4) == 4);
}

void test_utf8ChRunLenInvalid(void) {
    testAssert(utf8ChRunLen(0x80) == 0);
    testAssert(utf8ChRunLen(0xc0) == 0);
    testAssert(utf8ChRunLen(0xc1) == 0);
    testAssert(utf8ChRunLen(0xf5) == 0);
    testAssert(utf8ChRunLen(0xf8) == 0);
    testAssert(utf8ChRunLen(0xff) == 0);
}

testList(
    testMake(test_utf8ChRunLen1Byte),
    testMake(test_utf8ChRunLen2Bytes),
    testMake(test_utf8ChRunLen3Bytes),
    testMake(test_utf8ChRunLen4Bytes),
    testMake(test_utf8ChRunLenInvalid)
)
