#include "nv_test.h"
#include "nv_context.c"
#include "nv_unicode.h"

void test_ctxBufRemoveEmpty(void) {
    CtxBuf buf = { 0 };

    ctxBufRemove_(&buf, 1);
    testAssert(buf.len == 0);
    testAssert(buf.gapIdx == 0);

    memFree(buf.bytes);
}

void test_ctxBufRemoveFromEnd(void) {
    CtxBuf buf = { 0 };
    const char s[] = "abcd";

    ctxBufInsert_(&buf, (UcdCh8 *)s, chArrLen(s));

    ctxBufRemove_(&buf, 2);
    testAssert(buf.len == 2);
    testAssert(*ctxBufGet_(&buf, 0) == 'a');
    testAssert(*ctxBufGet_(&buf, 1) == 'b');

    memFree(buf.bytes);
}

void test_ctxBufRemoveFromMiddle(void) {
    CtxBuf buf = { 0 };
    const char s[] = "abcd";

    ctxBufInsert_(&buf, (UcdCh8 *)s, chArrLen(s));
    ctxBufSetGapIdx_(&buf, 2);
    ctxBufRemove_(&buf, 2);
    testAssert(buf.len == 2);
    testAssert(*ctxBufGet_(&buf, 0) == 'c');
    testAssert(*ctxBufGet_(&buf, 1) == 'd');

    memFree(buf.bytes);
}

testList(
    testMake(test_ctxBufRemoveEmpty),
    testMake(test_ctxBufRemoveFromEnd),
    testMake(test_ctxBufRemoveFromMiddle)
)
