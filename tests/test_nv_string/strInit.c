#include <string.h>

#include "nv_test.h"
#include "nv_string.h"

void test_strInitZeroReserve(void) {
    Str str;

    testAssertRequire(strInit(&str, 0));
    testAssert(str.len == 0);
    testAssert(strcmp(strAsC(&str), "") == 0);

    strDestroy(&str);
}

void test_strInitWithReserve(void) {
    size_t reserve = 10;
    Str str;

    testAssertRequire(strInit(&str, reserve));
    testAssert(str.len == 0);
    testAssert(str.cap >= reserve);
    testAssert(strcmp(strAsC(&str), "") == 0);

    strDestroy(&str);
}

testList(
    testMake(test_strInitZeroReserve),
    testMake(test_strInitWithReserve)
)
