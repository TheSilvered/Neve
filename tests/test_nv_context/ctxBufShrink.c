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

testList(
    testMake(test_ctxBufShrinkNoEffect),
    testMake(test_ctxBufShrinkEmpty)
)

