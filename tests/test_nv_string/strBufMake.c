#include <string.h>

#include "nv_test.h"
#include "nv_string.h"

void test_strBufMake(void) {
#define BUF_SIZE 10
    char buf[BUF_SIZE];
    StrBuf sb = strBufMake(buf, BUF_SIZE);

    testAssert(sb.buf == buf);
    testAssert(sb.len == 0);
    testAssert(sb.bufSize == BUF_SIZE);
#undef BUF_SIZE
}

testList(
    testMake(test_strBufMake)
)
