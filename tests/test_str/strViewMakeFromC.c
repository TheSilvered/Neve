#include <string.h>

#include "nv_test.h"
#include "nv_string.h"

void test_strViewMakeFromCEmpty(void) {
    const char *cStr = "";
    StrView sv = strViewMakeFromC(cStr);

    testAssert(strcmp((char *)sv.buf, cStr) == 0);
    testAssert(sv.len == 0);
}

void test_strViewMakeFromCFull(void) {
    const char *cStr = "a";
    StrView sv = strViewMakeFromC(cStr);

    testAssert(strcmp((char *)sv.buf, cStr) == 0);
    testAssert(sv.len == 1);
}

testList(
    testMake(test_strViewMakeFromCEmpty),
    testMake(test_strViewMakeFromCFull)
)
