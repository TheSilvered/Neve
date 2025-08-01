#include "nv_test.h"
#include "nv_unicode.h"

void test_ucdCh8RunLen1Byte(void) {
    testAssert(ucdCh8RunLen(0x0) == 1);
    testAssert(ucdCh8RunLen('a') == 1);
}

void test_ucdCh8RunLen2Bytes(void) {
    testAssert(ucdCh8RunLen(0xc2) == 2);
    testAssert(ucdCh8RunLen(0xdf) == 2);
}

void test_ucdCh8RunLen3Bytes(void) {
    testAssert(ucdCh8RunLen(0xe0) == 3);
    testAssert(ucdCh8RunLen(0xef) == 3);
}

void test_ucdCh8RunLen4Bytes(void) {
    testAssert(ucdCh8RunLen(0xf0) == 4);
    testAssert(ucdCh8RunLen(0xf4) == 4);
}

void test_ucdCh8RunLenInvalid(void) {
    testAssert(ucdCh8RunLen(0x80) == 0);
    testAssert(ucdCh8RunLen(0xc0) == 0);
    testAssert(ucdCh8RunLen(0xc1) == 0);
    testAssert(ucdCh8RunLen(0xf5) == 0);
    testAssert(ucdCh8RunLen(0xf8) == 0);
    testAssert(ucdCh8RunLen(0xff) == 0);
}

testList(
    testMake(test_ucdCh8RunLen1Byte),
    testMake(test_ucdCh8RunLen2Bytes),
    testMake(test_ucdCh8RunLen3Bytes),
    testMake(test_ucdCh8RunLen4Bytes),
    testMake(test_ucdCh8RunLenInvalid)
)
