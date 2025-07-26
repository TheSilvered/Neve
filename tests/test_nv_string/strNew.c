#include <string.h>

#include "nv_test.h"
#include "nv_string.h"

void test_strNewZeroReserve(void) {
    Str *str = strNew(0);

    testAssert(str->len == 0);
    testAssert(strcmp(strAsC(str), "") == 0);

    strFree(str);
}

void test_strNewWithReserve(void) {
    size_t reserve = 10;
    Str *str = strNew(reserve);

    testAssert(str->len == 0);
    testAssert(str->cap >= reserve);
    testAssert(strcmp(strAsC(str), "") == 0);

    strFree(str);
}

testList(
    testMake(test_strNewZeroReserve),
    testMake(test_strNewWithReserve)
)
