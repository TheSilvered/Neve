#include <string.h>

#include "nv_test.h"
#include "nv_string.h"

void test_strClearEmptyZeroReserve(void) {
    Str str;
    strInit(&str, 0);

    strClear(&str, 0);
    testAssert(strcmp(strAsC(&str), "") == 0);
    testAssert(str.len == 0);

    strDestroy(&str);
}

void test_strClearFullZeroReserve(void) {
    Str str;

    strInitFromC(&str, "a");
    strClear(&str, 0);
    testAssert(strcmp(strAsC(&str), "") == 0);
    testAssert(str.len == 0);

    strDestroy(&str);
}

void test_strClearEmptyWithReserve(void) {
    size_t reserve = 10;
    Str str;
    strInit(&str, 0);

    strClear(&str, reserve);
    testAssert(strcmp(strAsC(&str), "") == 0);
    testAssert(str.len == 0);
    testAssert(str.cap >= reserve);

    strDestroy(&str);
}

void test_strClearFullWithReserve(void) {
    size_t reserve = 10;
    Str str;

    strInitFromC(&str, "a");
    strClear(&str, reserve);
    testAssert(strcmp(strAsC(&str), "") == 0);
    testAssert(str.len == 0);
    testAssert(str.cap >= reserve);

    strDestroy(&str);
}

testList(
    testMake(test_strClearEmptyZeroReserve),
    testMake(test_strClearFullZeroReserve),
    testMake(test_strClearEmptyWithReserve),
    testMake(test_strClearFullWithReserve)
)
