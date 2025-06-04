#include <string.h>

#include "nv_test.h"
#include "nv_str.h"

void test_strNewFromCEmpty(void) {
    const char *cStr = "";
    Str *str = strNewFromC(cStr);

    testAssertRequire(str != NULL);
    testAssert(str->len == 0);
    testAssert(strcmp(strAsC(str), cStr));

    strFree(str);
}

void test_strNewFromCFull(void) {
    const char *cStr = "a";
    Str *str = strNewFromC(cStr);

    testAssertRequire(str != NULL);
    testAssert(str->len == 0);
    testAssert(strcmp(strAsC(str), cStr));

    strFree(str);
}

testList(
    testMake(test_strNewFromCEmpty),
    testMake(test_strNewFromCFull)
)
