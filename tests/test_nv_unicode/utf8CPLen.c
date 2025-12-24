#include "nv_test.h"
#include "unicode/nv_utf.h"

void test_utf8CPLenU0000(void) {
    testAssert(utf8CPLen(0x0000) == 1);
}

void test_utf8CPLenU007f(void) {
    testAssert(utf8CPLen(0x007f) == 1);
}

void test_utf8CPLenU0080(void) {
    testAssert(utf8CPLen(0x0080) == 2);
}

void test_utf8CPLenU07ff(void) {
    testAssert(utf8CPLen(0x07ff) == 2);
}

void test_utf8CPLenU0800(void) {
    testAssert(utf8CPLen(0x0800) == 3);
}

void test_utf8CPLenUffff(void) {
    testAssert(utf8CPLen(0xffff) == 3);
}

void test_utf8CPLenU100000(void) {
    testAssert(utf8CPLen(0x100000) == 4);
}

void test_utf8CPLenU10ffff(void) {
    testAssert(utf8CPLen(0x10ffff) == 4);
}

void test_utf8CPLenInvalid(void) {
    testAssert(utf8CPLen(0x110000) == 0);
    testAssert(utf8CPLen(-1) == 0);
    testAssert(utf8CPLen(ucdHighSurrogateFirst) == 0);
    testAssert(utf8CPLen(ucdHighSurrogateLast) == 0);
    testAssert(utf8CPLen(ucdLowSurrogateFirst) == 0);
    testAssert(utf8CPLen(ucdLowSurrogateLast) == 0);
}

testList(
    testMake(test_utf8CPLenU0000),
    testMake(test_utf8CPLenU007f),
    testMake(test_utf8CPLenU0080),
    testMake(test_utf8CPLenU07ff),
    testMake(test_utf8CPLenU0800),
    testMake(test_utf8CPLenUffff),
    testMake(test_utf8CPLenUffff),
    testMake(test_utf8CPLenUffff),
    testMake(test_utf8CPLenU100000),
    testMake(test_utf8CPLenU10ffff),
    testMake(test_utf8CPLenInvalid)
)
