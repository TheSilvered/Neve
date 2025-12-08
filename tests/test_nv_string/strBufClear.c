#include <string.h>

#include "nv_test.h"
#include "nv_string.h"

void test_strBufClearEmpty(void) {
    char buf[10];
    StrBuf sb = strBufMake(buf, nvArrlen(buf));

    strBufClear(&sb);
    testAssert(strcmp(sb.buf, "") == 0);
    testAssert(sb.len == 0);
}

void test_strBufClearFull(void) {
    char buf[10] = "a";
    StrBuf sb = {
        .buf = buf,
        .len = 1,
        .bufSize = nvArrlen(buf)
    };

    strBufClear(&sb);
    testAssert(strcmp(sb.buf, "") == 0);
    testAssert(sb.len == 0);
}

testList(
    testMake(test_strBufClearEmpty),
    testMake(test_strBufClearFull)
)
