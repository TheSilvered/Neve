#include <string.h>

#include "nv_test.h"
#include "nv_str.h"

void test_AppendEmpty(void) {
    Str str;
    strInit(&str, 0);
    StrView emptyView = strViewMakeFromC("");

    testAssert(strAppend(&str, &emptyView));
    testAssert(str.len == 0);
    testAssert(strcmp(strAsC(&str), "") == 0);

    strDestroy(&str);
}

void test_AppendEmptyToExisting(void) {
    Str str;
    testAssertRequire(strInitFromC(&str, "a"));
    StrView emptyView = strViewMakeFromC("");

    testAssert(strAppend(&str, &emptyView));
    testAssert(str.len == 1);
    testAssert(strcmp(strAsC(&str), "a") == 0);

    strDestroy(&str);
}

void test_AppendFull(void) {
    Str str;
    strInit(&str, 0);
    StrView fullView = strViewMakeFromC("a");

    testAssert(strAppend(&str, &fullView));
    testAssert(str.len == 1);
    testAssert(strcmp(strAsC(&str), "a") == 0);

    strDestroy(&str);
}

void test_AppendFullToExisting(void) {
    Str str;
    testAssertRequire(strInitFromC(&str, "a"));
    StrView fullView = strViewMakeFromC("b");

    testAssert(strAppend(&str, &fullView));
    testAssert(str.len == 2);
    testAssert(strcmp(strAsC(&str), "ab") == 0);

    strDestroy(&str);
}

testList(
    testMake(test_AppendEmpty),
    testMake(test_AppendEmptyToExisting),
    testMake(test_AppendFull),
    testMake(test_AppendFullToExisting)
)
