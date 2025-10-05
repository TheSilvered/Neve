#include "nv_test.h"
#include "nv_context.c"

void test_ctxBufReserveZero(void) {
    CtxBuf buf = { 0 };
    ctxBufReserve_(&buf, 0);

    testAssert(buf.cap == 0);
    testAssert(buf.len == 0);
    testAssert(buf.bytes == NULL);
}

void test_ctxBufReserveNonZero(void) {
    CtxBuf buf = { 0 };
    const size_t amount = 10;
    ctxBufReserve_(&buf, amount);

    testAssert(buf.cap >= amount);
    testAssert(buf.len == 0);
    testAssert(buf.bytes != NULL);

    memFree(buf.bytes);
}

void test_ctxBufReserveAlreadySatisfied(void) {
    CtxBuf buf = { 0 };
    const size_t amount = 10;
    ctxBufReserve_(&buf, amount);

    size_t prevCap = buf.cap;
    size_t prevLen = buf.len;
    UcdCh8 *prevBytes = buf.bytes;

    ctxBufReserve_(&buf, amount);

    testAssert(buf.cap >= amount);
    testAssert(buf.len == 0);
    testAssert(buf.bytes != NULL);

    memFree(buf.bytes);
}

testList(
    testMake(test_ctxBufReserveZero)
)

