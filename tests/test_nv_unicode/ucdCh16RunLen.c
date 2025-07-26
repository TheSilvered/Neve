#include "nv_test.h"
#include "nv_unicode.h"

void test_ucdCh16RunLenBMP(void) {
    testAssert(ucdCh16RunLen(0x0000) == 1);
    testAssert(ucdCh16RunLen(0xffff) == 1);
}

void test_ucdCh16RunLenHighSurrogate(void) {
    testAssert(ucdCh16RunLen(ucdHighSurrogateFirst) == 2);
    testAssert(ucdCh16RunLen(ucdHighSurrogateLast) == 2);
}

void test_ucdCh16RunLenLowSurrogate(void) {
    testAssert(ucdCh16RunLen(ucdLowSurrogateFirst) == 0);
    testAssert(ucdCh16RunLen(ucdLowSurrogateLast) == 0);
}

testList(
    testMake(test_ucdCh16RunLenBMP),
    testMake(test_ucdCh16RunLenHighSurrogate),
    testMake(test_ucdCh16RunLenLowSurrogate)
)
