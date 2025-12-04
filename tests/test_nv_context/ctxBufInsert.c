#include "nv_test.h"
#include "nv_context.c"

void test_ctxBufInsertEmpty(void) {
    CtxBuf buf = { 0 };

    ctxBufInsert_(&buf, NULL, 0);
    testAssert(buf.len == 0);
    testAssert(buf.cap == 0);
    testAssert(buf.bytes == NULL);

    memFree(buf.bytes);
}

void test_ctxBufInsertFromEmpty(void) {
    CtxBuf buf = { 0 };
    const char s[] = "abcd";

    ctxBufInsert_(&buf, (UcdCh8 *)s, chArrLen(s));
    testAssert(buf.len == chArrLen(s));
    testAssert(buf.cap >= chArrLen(s));
    testAssert(buf.gapIdx == chArrLen(s));

    memFree(buf.bytes);
}

void test_ctxBufInsertFromEnd(void) {
    CtxBuf buf = { 0 };
    const char s[] = "abcd";

    ctxBufInsert_(&buf, (UcdCh8 *)s, chArrLen(s));
    testAssert(buf.gapIdx == chArrLen(s));
    ctxBufInsert_(&buf, (UcdCh8 *)s, chArrLen(s));
    testAssert(buf.len == chArrLen(s) * 2);
    testAssert(buf.cap >= chArrLen(s) * 2);
    testAssert(buf.gapIdx == chArrLen(s) * 2);

    memFree(buf.bytes);
}

testList(
    testMake(test_ctxBufInsertEmpty),
    testMake(test_ctxBufInsertFromEmpty),
    testMake(test_ctxBufInsertFromEnd)
)
