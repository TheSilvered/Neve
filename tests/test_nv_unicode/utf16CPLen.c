#include "nv_test.h"
#include "unicode/nv_utf.h"

void test_utf16CPLenBMP(void) {
    testAssert(utf16CPLen(0x0000) == 1);
    testAssert(utf16CPLen(0xffff) == 1);
}

void test_utf16CPLenSupplementaryPlanes(void) {
    testAssert(utf16CPLen(0x10000) == 2);
    testAssert(utf16CPLen(0x10ffff) == 2);
}

void test_utf16CPLenInvalid(void) {
    testAssert(utf16CPLen(-1) == 0);
    testAssert(utf16CPLen(0x110000) == 0);
    testAssert(utf16CPLen(ucdHighSurrogateFirst) == 0);
    testAssert(utf16CPLen(ucdHighSurrogateLast) == 0);
    testAssert(utf16CPLen(ucdLowSurrogateFirst) == 0);
    testAssert(utf16CPLen(ucdLowSurrogateLast) == 0);
}

testList(
    testMake(test_utf16CPLenBMP),
    testMake(test_utf16CPLenSupplementaryPlanes),
    testMake(test_utf16CPLenInvalid)
)
