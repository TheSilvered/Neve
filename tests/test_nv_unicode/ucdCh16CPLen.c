#include "nv_test.h"
#include "nv_unicode.h"

void test_ucdCh16CPLenBMP(void) {
    testAssert(ucdCh16CPLen(0x0000) == 1);
    testAssert(ucdCh16CPLen(0xffff) == 1);
}

void test_ucdCh16CPLenSupplementaryPlanes(void) {
    testAssert(ucdCh16CPLen(0x10000) == 2);
    testAssert(ucdCh16CPLen(0x10ffff) == 2);
}

void test_ucdCh16CPLenInvalid(void) {
    testAssert(ucdCh16CPLen(-1) == 0);
    testAssert(ucdCh16CPLen(0x110000) == 0);
    testAssert(ucdCh16CPLen(ucdHighSurrogateFirst) == 0);
    testAssert(ucdCh16CPLen(ucdHighSurrogateLast) == 0);
    testAssert(ucdCh16CPLen(ucdLowSurrogateFirst) == 0);
    testAssert(ucdCh16CPLen(ucdLowSurrogateLast) == 0);
}

testList(
    testMake(test_ucdCh16CPLenBMP),
    testMake(test_ucdCh16CPLenSupplementaryPlanes),
    testMake(test_ucdCh16CPLenInvalid)
)
