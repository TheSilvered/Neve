#include "nv_test.h"
#include "nv_context.c"

void test_ctxBufSetGapIdxZero(void) {
    CtxBuf buf = { 0 };
    const char s[] = "abcd";

    ctxBufInsert_(&buf, (UcdCh8 *)s, chArrLen(s));
    ctxBufSetGapIdx_(&buf, 0);

    testAssert(buf.gapIdx == 0);
    testAssert(buf.bytes[buf.cap - 1] == 'd');
    testAssert(buf.bytes[buf.cap - 2] == 'c');
    testAssert(buf.bytes[buf.cap - 3] == 'b');
    testAssert(buf.bytes[buf.cap - 4] == 'a');

    memFree(buf.bytes);
}

void test_ctxBufSetGapIdxEnd(void) {
    CtxBuf buf = { 0 };
    const char s[] = "abcd";

    ctxBufInsert_(&buf, (UcdCh8 *)s, chArrLen(s));
    ctxBufSetGapIdx_(&buf, 0);
    ctxBufSetGapIdx_(&buf, buf.len);

    testAssert(buf.gapIdx == buf.len);
    testAssert(buf.bytes[0] == 'a');
    testAssert(buf.bytes[1] == 'b');
    testAssert(buf.bytes[2] == 'c');
    testAssert(buf.bytes[3] == 'd');

    memFree(buf.bytes);
}

testList(
    testMake(test_ctxBufSetGapIdxZero),
    testMake(test_ctxBufSetGapIdxEnd)
)
