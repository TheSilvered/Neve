#include "nv_test.h"
#include "nv_unicode.h"

void test_ucdCh16RunLenBMP(void) {
    testAssert(ucdCh16RunLen(0x0000) == 1);
    testAssert(ucdCh16RunLen(0xffff) == 1);
}

void test_ucdCh16RunLenHighSurrogate(void) {
    testAssert(ucdCh16RunLen(UCD_HIGH_SURROGATE_FIRST) == 2);
    testAssert(ucdCh16RunLen(UCD_HIGH_SURROGATE_LAST) == 2);
}

void test_ucdCh16RunLenLowSurrogate(void) {
    testAssert(ucdCh16RunLen(UCD_LOW_SURROGATE_FIRST) == 0);
    testAssert(ucdCh16RunLen(UCD_LOW_SURROGATE_LAST) == 0);
}

testList(
    testMake(test_ucdCh16RunLenBMP),
    testMake(test_ucdCh16RunLenHighSurrogate),
    testMake(test_ucdCh16RunLenLowSurrogate)
)
