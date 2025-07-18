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

void test_ucdCh8StrToCh16StrSizeU007f(void) {
    const UcdCh8 str[1] = { 0x7f };
    testAssert(ucdCh8StrToCh16Str(str, 1, NULL, 0) == 2);
}

void test_ucdCh8StrToCh16StrU007f(void) {
    UcdCh16 buf[2] = { 0xffff, 0xffff };
    const UcdCh8 str[1] = { 0x7f };

    testAssertRequire(ucdCh8StrToCh16Str(str, 1, buf, 2) == 1);
    testAssert(buf[0] == 0x007f);
}

void test_ucdCh8StrToCh16StrSizeU0000U007f(void) {
    const UcdCh8 str[2] = { 0x00, 0x7f };
    testAssert(ucdCh8StrToCh16Str(str, 2, NULL, 0) == 3);
}

void test_ucdCh8StrToCh16StrU0000U007f(void) {
    UcdCh16 buf[3] = { 0xffff, 0xffff, 0xffff };
    const UcdCh8 str[2] = { 0x00, 0x7f };

    testAssertRequire(ucdCh8StrToCh16Str(str, 2, buf, 3) == 2);
    testAssert(buf[0] == 0x0000);
    testAssert(buf[1] == 0x007f);
    testAssert(buf[2] == 0x0000);
}

void test_ucdCh8StrToCh16StrSizeU0080(void) {
    const UcdCh8 str[2] = { 0xc2, 0x80 };
    testAssert(ucdCh8StrToCh16Str(str, 2, NULL, 0) == 2);
}

void test_ucdCh8StrToCh16StrU0080(void) {
    UcdCh16 buf[2] = { 0xffff, 0xffff };
    const UcdCh8 str[2] = { 0xc2, 0x80 };

    testAssertRequire(ucdCh8StrToCh16Str(str, 2, buf, 2) == 1);
    testAssert(buf[0] == 0x0080);
    testAssert(buf[1] == 0x0000);
}

void test_ucdCh8StrToCh16StrSizeU07ff(void) {
    const UcdCh8 str[2] = { 0xdf, 0xbf };
    testAssert(ucdCh8StrToCh16Str(str, 2, NULL, 0) == 2);
}

void test_ucdCh8StrToCh16StrU07ff(void) {
    UcdCh16 buf[2] = { 0xffff, 0xffff };
    const UcdCh8 str[2] = { 0xdf, 0xbf };

    testAssertRequire(ucdCh8StrToCh16Str(str, 2, buf, 2) == 1);
    testAssert(buf[0] == 0x07ff);
    testAssert(buf[1] == 0x0000);
}

void test_ucdCh8StrToCh16StrSizeU0800(void) {
    const UcdCh8 str[3] = { 0xe0, 0xa0, 0x80 };
    testAssert(ucdCh8StrToCh16Str(str, 3, NULL, 0) == 2);
}

void test_ucdCh8StrToCh16StrU0800(void) {
    UcdCh16 buf[2] = { 0xffff, 0xffff };
    const UcdCh8 str[3] = { 0xe0, 0xa0, 0x80 };

    testAssertRequire(ucdCh8StrToCh16Str(str, 3, buf, 2) == 1);
    testAssert(buf[0] == 0x0800);
    testAssert(buf[1] == 0x0000);
}

testList(
    testMake(test_ucdCh8StrToCh16StrSizeU0000),
    testMake(test_ucdCh8StrToCh16StrU0000),
    testMake(test_ucdCh8StrToCh16StrSizeU007f),
    testMake(test_ucdCh8StrToCh16StrU007f),
    testMake(test_ucdCh8StrToCh16StrSizeU0000U007f),
    testMake(test_ucdCh8StrToCh16StrU0000U007f),
    testMake(test_ucdCh8StrToCh16StrSizeU0080),
    testMake(test_ucdCh8StrToCh16StrU0080),
    testMake(test_ucdCh8StrToCh16StrSizeU07ff),
    testMake(test_ucdCh8StrToCh16StrU07ff),
    testMake(test_ucdCh8StrToCh16StrSizeU0800),
    testMake(test_ucdCh8StrToCh16StrU0800)
);
