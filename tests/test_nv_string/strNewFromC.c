#include <string.h>

#include "nv_test.h"
#include "nv_string.h"

void test_strNewFromCEmpty(void) {
    const char *cStr = "";
    Str *str = strNewFromC(cStr);

    testAssert(str->len == 0);
    testAssert(strcmp(strAsC(str), cStr) == 0);

    strFree(str);
}

void test_strNewFromCFull(void) {
    const char *cStr = "a";
    Str *str = strNewFromC(cStr);

    testAssert(str->len == 1);
    testAssert(strcmp(strAsC(str), cStr) == 0);

    strFree(str);
}

testList(
    testMake(test_strNewFromCEmpty),
    testMake(test_strNewFromCFull)
)
