#include "nv_test.h"
#include "nv_context.c"

void test_ctxBufShrinkNoEffect(void) {
    CtxBuf buf = { 0 };
    const char s[] = "abcd";
    _ctxBufInsert(&buf, (UcdCh8 *)s, chArrLen(s));

    _ctxBufShrink(&buf);
    testAssert(buf.len == 4);
    testAssert(buf.cap >= 4);

    memFree(buf.bytes);
}

void test_ctxBufShrinkEmpty(void) {
    CtxBuf buf = { 0 };

    _ctxBufReserve(&buf, 1000);
    size_t prevCap = buf.cap;

    _ctxBufShrink(&buf);

    testAssert(buf.cap < prevCap);
    memFree(buf.bytes);
}

void test_ctxShrkinkFull(void) {
    CtxBuf buf = { 0 };
    const char s[] = "abcd";

    _ctxBufReserve(&buf, 1000);
    _ctxBufInsert(&buf, (UcdCh8 *)s, chArrLen(s));
    _ctxBufSetGapIdx(&buf, 2);

    size_t prevCap = buf.cap;

    _ctxBufShrink(&buf);

    testAssert(buf.cap < prevCap);
    testAssert(*_ctxBufGet(&buf, 0) == s[0]);
    testAssert(*_ctxBufGet(&buf, 1) == s[1]);
    testAssert(*_ctxBufGet(&buf, 2) == s[2]);
    testAssert(*_ctxBufGet(&buf, 3) == s[3]);

    memFree(buf.bytes);
}

testList(
    testMake(test_ctxBufShrinkNoEffect),
    testMake(test_ctxBufShrinkEmpty),
    testMake(test_ctxShrkinkFull)
)
