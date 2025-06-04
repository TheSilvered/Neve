#include <string.h>

#include "nv_test.h"
#include "nv_str.h"

void test_strNewZeroReserve(void) {
    Str *str = strNew(0);

    testAssertRequire(str != NULL);
    testAssert(str->len == 0);
    testAssert(strcmp(strAsC(str), ""));

    strFree(str);
}

void test_strNewNonZeroReserve(void) {
    size_t reserve = 10;
    Str *str = strNew(reserve);

    testAssertRequire(str != NULL);
    testAssert(str->len == 0);
    testAssert(str->cap >= reserve);
    testAssert(strcmp(strAsC(str), ""));

    strFree(str);
}

testList(
    testMake(test_strNewZeroReserve),
    testMake(test_strNewNonZeroReserve)
)
