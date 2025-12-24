#include "nv_test.h"
#include "unicode/nv_utf.h"

void test_utf16ChRunLenBMP(void) {
    testAssert(utf16ChRunLen(0x0000) == 1);
    testAssert(utf16ChRunLen(0xffff) == 1);
}

void test_utf16ChRunLenHighSurrogate(void) {
    testAssert(utf16ChRunLen(ucdHighSurrogateFirst) == 2);
    testAssert(utf16ChRunLen(ucdHighSurrogateLast) == 2);
}

void test_utf16ChRunLenLowSurrogate(void) {
    testAssert(utf16ChRunLen(ucdLowSurrogateFirst) == 0);
    testAssert(utf16ChRunLen(ucdLowSurrogateLast) == 0);
}

testList(
    testMake(test_utf16ChRunLenBMP),
    testMake(test_utf16ChRunLenHighSurrogate),
    testMake(test_utf16ChRunLenLowSurrogate)
)
