#include "nv_test.h"
#include "nv_unicode.h"

void test_ucdCh8StrToCh16StrSizeU0000(void) {
    const UcdCh8 str[1] = { 0x00 };
    testAssert(ucdCh8StrToCh16Str(str, 1, NULL, 0) == 2);
}

void test_ucdCh8StrToCh16StrU0000(void) {
    UcdCh16 buf[2] = { 0xffff, 0xffff };
    const UcdCh8 str[1] = { 0x00 };
    testAssert(ucdCh8StrToCh16Str(str, 1, buf, 2) == 1);
    testAssert(buf[0] == 0x0000);
    testAssert(buf[1] == 0x0000);
}

testList(
    testMake(test_ucdCh8StrToCh16StrSizeU0000),
    testMake(test_ucdCh8StrToCh16StrU0000)
);