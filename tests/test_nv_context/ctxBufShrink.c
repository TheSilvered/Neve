#include "nv_test.h"
#include "nv_context.c"

void test_ctxBufShrinkNoEffect(void) {
    CtxBuf buf = { 0 };
    const char s[] = "abcd";
    ctxBufInsert_(&buf, (UcdCh8 *)s, chArrLen(s));

    ctxBufShrink_(&buf);
    testAssert(buf.len == 4);
    testAssert(buf.cap >= 4);

    memFree(buf.bytes);
}

void test_ctxBufShrinkEmpty(void) {
    CtxBuf buf = { 0 };

    ctxBufReserve_(&buf, 1000);
    size_t prevCap = buf.cap;

    ctxBufShrink_(&buf);

    testAssert(buf.cap < prevCap);
    memFree(buf.bytes);
}

void test_ctxShrkinkFull(void) {
    CtxBuf buf = { 0 };
    const char s[] = "abcd";

    ctxBufReserve_(&buf, 1000);
    ctxBufInsert_(&buf, (UcdCh8 *)s, chArrLen(s));
    ctxBufSetGapIdx_(&buf, 2);

    size_t prevCap = buf.cap;

    ctxBufShrink_(&buf);

    testAssert(buf.cap < prevCap);
    testAssert(*ctxBufGet_(&buf, 0) == s[0]);
    testAssert(*ctxBufGet_(&buf, 1) == s[1]);
    testAssert(*ctxBufGet_(&buf, 2) == s[2]);
    testAssert(*ctxBufGet_(&buf, 3) == s[3]);
}

testList(
    testMake(test_ctxBufShrinkNoEffect),
    testMake(test_ctxBufShrinkEmpty),
    testMake(test_ctxShrkinkFull)
)
