#include <string.h>

#include "nv_test.h"
#include "nv_string.h"

void test_strAsCEmptyZeroReserve(void) {
    Str str;
    strInit(&str, 0);

    testAssert(strcmp(strAsC(&str), "") == 0);

    strDestroy(&str);
}

void test_strAsCEmptyWithReserve(void) {
    Str str;

    strInit(&str, 10);
    testAssert(strcmp(strAsC(&str), "") == 0);

    strDestroy(&str);
}

void test_strAsCFull(void) {
    const char *cStr = "a";
    Str str;

    strInitFromC(&str, cStr);
    testAssert(strcmp(strAsC(&str), cStr) == 0);

    strDestroy(&str);
}

testList(
    testMake(test_strAsCEmptyZeroReserve),
    testMake(test_strAsCEmptyWithReserve),
    testMake(test_strAsCFull)
)
