#include <string.h>

#include "nv_test.h"
#include "nv_string.h"

void test_strBufMake(void) {
    char buf[10];
    StrBuf sb = strBufMake(buf, nvArrlen(buf));

    testAssert(sb.buf == buf);
    testAssert(sb.len == 0);
    testAssert(sb.bufSize == nvArrlen(buf));
}

testList(
    testMake(test_strBufMake)
)
