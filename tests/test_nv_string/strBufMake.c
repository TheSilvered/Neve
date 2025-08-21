#include <string.h>

#include "nv_test.h"
#include "nv_string.h"

void test_strBufMake(void) {
    char buf[10];
    StrBuf sb = strBufMake(buf, NV_ARRLEN(buf));

    testAssert(sb.buf == buf);
    testAssert(sb.len == 0);
    testAssert(sb.bufSize == NV_ARRLEN(buf));
}

testList(
    testMake(test_strBufMake)
)
