#include <string.h>

#include "nv_test.h"
#include "nv_string.h"

void test_strAppendEmpty(void) {
    Str str;
    strInit(&str, 0);
    StrView emptyView = strViewMakeFromC("");

    strAppend(&str, &emptyView);
    testAssert(str.len == 0);
    testAssert(strcmp(strAsC(&str), "") == 0);

    strDestroy(&str);
}

void test_strAppendEmptyToExisting(void) {
    Str str;
    strInitFromC(&str, "a");
    StrView emptyView = strViewMakeFromC("");

    strAppend(&str, &emptyView);
    testAssert(str.len == 1);
    testAssert(strcmp(strAsC(&str), "a") == 0);

    strDestroy(&str);
}

void test_strAppendFull(void) {
    Str str;
    strInit(&str, 0);
    StrView fullView = strViewMakeFromC("a");

    strAppend(&str, &fullView);
    testAssert(str.len == 1);
    testAssert(strcmp(strAsC(&str), "a") == 0);

    strDestroy(&str);
}

void test_strAppendFullToExisting(void) {
    Str str;
    strInitFromC(&str, "a");
    StrView fullView = strViewMakeFromC("b");

    strAppend(&str, &fullView);
    testAssert(str.len == 2);
    testAssert(strcmp(strAsC(&str), "ab") == 0);

    strDestroy(&str);
}

testList(
    testMake(test_strAppendEmpty),
    testMake(test_strAppendEmptyToExisting),
    testMake(test_strAppendFull),
    testMake(test_strAppendFullToExisting)
)
