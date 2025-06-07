#include <string.h>

#include "nv_test.h"
#include "nv_str.h"

#define BUF_SIZE 10

void test_strBufClearEmpty(void) {
    char buf[BUF_SIZE];
    StrBuf sb = strBufMake(buf, BUF_SIZE);

    strBufClear(&sb);
    testAssert(strcmp(sb.buf, "") == 0);
    testAssert(sb.len == 0);
}

void test_strBufClearFull(void) {
    char buf[BUF_SIZE] = "a";
    StrBuf sb = {
        .buf = buf,
        .len = 1,
        .bufSize = BUF_SIZE
    };

    strBufClear(&sb);
    testAssert(strcmp(sb.buf, "") == 0);
    testAssert(sb.len == 0);
}

testList(
    testMake(test_strBufClearEmpty),
    testMake(test_strBufClearFull)
)
