#include "nv_test.h"
#include "nv_unicode.h"

void test_ucdCh16ToCPBMP(void) {
    const UcdCh16 arr0[1] = { 0x0000 };
    const UcdCh16 arr1[1] = { 0xffff };

    testAssert(ucdCh16ToCP(arr0) == 0x0000);
    testAssert(ucdCh16ToCP(arr1) == 0xffff);
}

void test_ucdCh16ToCPBMPSupplementaryPlanes(void) {
    const UcdCh16 arr0[2] = {
        ucdHighSurrogateFirst,
        ucdLowSurrogateFirst
    };
    const UcdCh16 arr1[2] = {
        ucdHighSurrogateLast,
        ucdLowSurrogateLast
    };

    testAssert(ucdCh16ToCP(arr0) == 0x10000);
    testAssert(ucdCh16ToCP(arr1) == 0x10ffff);
}

testList(
    testMake(test_ucdCh16ToCPBMP),
    testMake(test_ucdCh16ToCPBMP)
)
