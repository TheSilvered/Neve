#include "nv_test.h"
#include "nv_context.c"

void test_ctxBufGetBeforeGap(void) {
    CtxBuf buf = { 0 };
    const char s[] = "abcd";

    ctxBufInsert_(&buf, (UcdCh8 *)s, chArrLen(s));
    ctxBufSetGapIdx_(&buf, 2);

    testAssert(*ctxBufGet_(&buf, 0) == 'a');
    testAssert(*ctxBufGet_(&buf, 1) == 'b');

    memFree(buf.bytes);
}

void test_ctxBufGetAfterGap(void) {
    CtxBuf buf = { 0 };
    const char s[] = "abcd";

    ctxBufInsert_(&buf, (UcdCh8 *)s, chArrLen(s));
    ctxBufSetGapIdx_(&buf, 2);

    testAssert(*ctxBufGet_(&buf, 2) == 'c');
    testAssert(*ctxBufGet_(&buf, 3) == 'd');

    memFree(buf.bytes);
}

testList(
    testMake(test_ctxBufGetBeforeGap)
)
