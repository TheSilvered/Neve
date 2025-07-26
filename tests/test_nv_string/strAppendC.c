#include <string.h>

#include "nv_test.h"
#include "nv_string.h"

void test_strAppendCEmpty(void) {
    Str str;
    strInit(&str, 0);

    strAppendC(&str, "");
    testAssert(str.len == 0);
    testAssert(strcmp(strAsC(&str), "") == 0);

    strDestroy(&str);
}

void test_strAppendCEmptyToExisting(void) {
    Str str;
    strInitFromC(&str, "a");

    strAppendC(&str, "");
    testAssert(str.len == 1);
    testAssert(strcmp(strAsC(&str), "a") == 0);

    strDestroy(&str);
}

void test_strAppendCFull(void) {
    Str str;
    strInit(&str, 0);

    strAppendC(&str, "a");
    testAssert(str.len == 1);
    testAssert(strcmp(strAsC(&str), "a") == 0);

    strDestroy(&str);
}

void test_strAppendCFullToExisting(void) {
    Str str;
    strInitFromC(&str, "a");

    strAppendC(&str, "b");
    testAssert(str.len == 2);
    testAssert(strcmp(strAsC(&str), "ab") == 0);

    strDestroy(&str);
}

testList(
    testMake(test_strAppendCEmpty),
    testMake(test_strAppendCEmptyToExisting),
    testMake(test_strAppendCFull),
    testMake(test_strAppendCFullToExisting)
)
