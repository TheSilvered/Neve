#include "nv_test.h"
#include "nv_context.c"

void test_ctxBufReserveZero(void) {
    CtxBuf buf = { 0 };
    _ctxBufReserve(&buf, 0);

    testAssert(buf.cap == 0);
    testAssert(buf.len == 0);
    testAssert(buf.bytes == NULL);
}

void test_ctxBufReserveNonZero(void) {
    CtxBuf buf = { 0 };
    const size_t amount = 10;
    _ctxBufReserve(&buf, amount);

    testAssert(buf.cap >= amount);
    testAssert(buf.len == 0);
    testAssert(buf.bytes != NULL);

    memFree(buf.bytes);
}

void test_ctxBufReserveAlreadySatisfied(void) {
    CtxBuf buf = { 0 };
    const size_t amount = 10;
    _ctxBufReserve(&buf, amount);

    size_t prevCap = buf.cap;
    size_t prevLen = buf.len;
    Utf8Ch *prevBytes = buf.bytes;

    _ctxBufReserve(&buf, amount);

    testAssert(buf.cap >= amount);
    testAssert(buf.len == 0);
    testAssert(buf.bytes != NULL);

    memFree(buf.bytes);
}

testList(
    testMake(test_ctxBufReserveZero)
)
