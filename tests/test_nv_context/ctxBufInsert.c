#include "nv_test.h"
#include "nv_context.c"

void test_ctxBufInsertEmpty(void) {
    CtxBuf buf = { 0 };

    ctxBufInsert_(&buf, NULL, 0);
    testAssert(buf.len == 0);
    testAssert(buf.cap == 0);
    testAssert(buf.bytes == NULL);
}

void test_ctxBufInsertFromEmpty(void) {
    CtxBuf buf = { 0 };
    const char s[] = "abcd";

    ctxBufInsert_(&buf, (UcdCh8 *)s, chArrLen(s));
    testAssert(buf.len == chArrLen(s));
    testAssert(buf.cap >= chArrLen(s));
    testAssert(buf.gapIdx == chArrLen(s));
}

testList(
    testMake(test_ctxBufInsertEmpty),
    testMake(test_ctxBufInsertFromEmpty)
)

