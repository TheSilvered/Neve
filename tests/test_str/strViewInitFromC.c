#include <string.h>

#include "nv_test.h"
#include "nv_string.h"

void test_strViewInitFromCEmpty(void) {
    const char *cStr = "";
    StrView sv;
    strViewInitFromC(&sv, cStr);

    testAssert(strcmp((char *)sv.buf, cStr) == 0);
    testAssert(sv.len == 0);
}

void test_strViewInitFromCFull(void) {
    const char *cStr = "a";
    StrView sv;
    strViewInitFromC(&sv, cStr);

    testAssert(strcmp((char *)sv.buf, cStr) == 0);
    testAssert(sv.len == 1);
}

testList(
    testMake(test_strViewInitFromCEmpty),
    testMake(test_strViewInitFromCFull)
)
