#include <string.h>

#include "nv_test.h"
#include "nv_str.h"

void test_strInitFromCEmpty(void) {
    const char *cStr = "";
    Str str;

    testAssertRequire(strInitFromC(&str, cStr));
    testAssert(str.len == 0);
    testAssert(strcmp(strAsC(&str), cStr));

    strDestroy(&str);
}

void test_strInitFromCFull(void) {
    const char *cStr = "a";
    Str str;

    testAssertRequire(strInitFromC(&str, cStr));
    testAssert(str.len == 0);
    testAssert(strcmp(strAsC(&str), cStr));

    strDestroy(&str);
}

testList(
    testMake(test_strInitFromCEmpty),
    testMake(test_strInitFromCFull)
)
