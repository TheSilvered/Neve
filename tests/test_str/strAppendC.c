#include <string.h>

#include "nv_test.h"
#include "nv_str.h"

void test_AppendCEmpty(void) {
    Str str;
    strInit(&str, 0);

    testAssert(strAppendC(&str, ""));
    testAssert(str.len == 0);
    testAssert(strcmp(strAsC(&str), "") == 0);
}

void test_AppendCEmptyToExisting(void) {
    Str str;
    testAssertRequire(strInitFromC(&str, "a"));

    testAssert(strAppendC(&str, ""));
    testAssert(str.len == 1);
    testAssert(strcmp(strAsC(&str), "a") == 0);
}

void test_AppendCFull(void) {
    Str str;
    strInit(&str, 0);

    testAssert(strAppendC(&str, "a"));
    testAssert(str.len == 1);
    testAssert(strcmp(strAsC(&str), "a") == 0);
}

void test_AppendCFullToExisting(void) {
    Str str;
    testAssertRequire(strInitFromC(&str, "a"));

    testAssert(strAppendC(&str, "b"));
    testAssert(str.len == 2);
    testAssert(strcmp(strAsC(&str), "ab") == 0);
}

testList(
    testMake(test_AppendCEmpty),
    testMake(test_AppendCEmptyToExisting),
    testMake(test_AppendCFull),
    testMake(test_AppendCFullToExisting)
)
