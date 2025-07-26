#include "nv_test.h"
#include "nv_unicode.h"

void test_ucdCh8CPLenU0000(void) {
    testAssert(ucdCh8CPLen(0x0000) == 1);
}

void test_ucdCh8CPLenU007f(void) {
    testAssert(ucdCh8CPLen(0x007f) == 1);
}

void test_ucdCh8CPLenU0080(void) {
    testAssert(ucdCh8CPLen(0x0080) == 2);
}

void test_ucdCh8CPLenU07ff(void) {
    testAssert(ucdCh8CPLen(0x07ff) == 2);
}

void test_ucdCh8CPLenU0800(void) {
    testAssert(ucdCh8CPLen(0x0800) == 3);
}

void test_ucdCh8CPLenUffff(void) {
    testAssert(ucdCh8CPLen(0xffff) == 3);
}

void test_ucdCh8CPLenU100000(void) {
    testAssert(ucdCh8CPLen(0x100000) == 4);
}

void test_ucdCh8CPLenU10ffff(void) {
    testAssert(ucdCh8CPLen(0x10ffff) == 4);
}

void test_ucdCh8CPLenInvalid(void) {
    testAssert(ucdCh8CPLen(0x110000) == 0);
    testAssert(ucdCh8CPLen(-1) == 0);
    testAssert(ucdCh8CPLen(ucdHighSurrogateFirst) == 0);
    testAssert(ucdCh8CPLen(ucdHighSurrogateLast) == 0);
    testAssert(ucdCh8CPLen(ucdLowSurrogateFirst) == 0);
    testAssert(ucdCh8CPLen(ucdLowSurrogateLast) == 0);
}

testList(
    testMake(test_ucdCh8CPLenU0000),
    testMake(test_ucdCh8CPLenU007f),
    testMake(test_ucdCh8CPLenU0080),
    testMake(test_ucdCh8CPLenU07ff),
    testMake(test_ucdCh8CPLenU0800),
    testMake(test_ucdCh8CPLenUffff),
    testMake(test_ucdCh8CPLenUffff),
    testMake(test_ucdCh8CPLenUffff),
    testMake(test_ucdCh8CPLenU100000),
    testMake(test_ucdCh8CPLenU10ffff),
    testMake(test_ucdCh8CPLenInvalid)
)
