#include <string.h>

#include "nv_test.h"
#include "nv_string.h"

void test_strInitFromCEmpty(void) {
    const char *cStr = "";
    Str str;

    strInitFromC(&str, cStr);
    testAssert(str.len == 0);
    testAssert(strcmp(strAsC(&str), cStr) == 0);

    strDestroy(&str);
}

void test_strInitFromCFull(void) {
    const char *cStr = "a";
    Str str;

    strInitFromC(&str, cStr);
    testAssert(str.len == 1);
    testAssert(strcmp(strAsC(&str), cStr) == 0);

    strDestroy(&str);
}

testList(
    testMake(test_strInitFromCEmpty),
    testMake(test_strInitFromCFull)
)
