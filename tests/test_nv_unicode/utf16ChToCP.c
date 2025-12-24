#include "nv_test.h"
#include "unicode/nv_utf.h"

void test_utf16ChToCPBMP(void) {
    const Utf16Ch arr0[1] = { 0x0000 };
    const Utf16Ch arr1[1] = { 0xffff };

    testAssert(utf16ChToCP(arr0) == 0x0000);
    testAssert(utf16ChToCP(arr1) == 0xffff);
}

void test_utf16ChToCPBMPSupplementaryPlanes(void) {
    const Utf16Ch arr0[2] = {
        ucdHighSurrogateFirst,
        ucdLowSurrogateFirst
    };
    const Utf16Ch arr1[2] = {
        ucdHighSurrogateLast,
        ucdLowSurrogateLast
    };

    testAssert(utf16ChToCP(arr0) == 0x10000);
    testAssert(utf16ChToCP(arr1) == 0x10ffff);
}

testList(
    testMake(test_utf16ChToCPBMP),
    testMake(test_utf16ChToCPBMP)
)
