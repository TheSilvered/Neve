#include "nv_test.h"
#include "nv_context.c"

void test_ctxBufGetBeforeGap(void) {
    CtxBuf buf = { 0 };
    const char s[] = "abcd";

    _ctxBufInsert(&buf, (UcdCh8 *)s, chArrLen(s));
    _ctxBufSetGapIdx(&buf, 2);

    testAssert(*_ctxBufGet(&buf, 0) == 'a');
    testAssert(*_ctxBufGet(&buf, 1) == 'b');

    memFree(buf.bytes);
}

void test_ctxBufGetAfterGap(void) {
    CtxBuf buf = { 0 };
    const char s[] = "abcd";

    _ctxBufInsert(&buf, (UcdCh8 *)s, chArrLen(s));
    _ctxBufSetGapIdx(&buf, 2);

    testAssert(*_ctxBufGet(&buf, 2) == 'c');
    testAssert(*_ctxBufGet(&buf, 3) == 'd');

    memFree(buf.bytes);
}

testList(
    testMake(test_ctxBufGetBeforeGap)
)
