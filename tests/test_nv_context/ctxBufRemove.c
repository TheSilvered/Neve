#include "nv_test.h"
#include "nv_context.c"
#include "unicode/nv_utf.h"

void test_ctxBufRemoveEmpty(void) {
    CtxBuf buf = { 0 };

    _ctxBufRemove(&buf, 1);
    testAssert(buf.len == 0);
    testAssert(buf.gapIdx == 0);

    memFree(buf.bytes);
}

void test_ctxBufRemoveFromEnd(void) {
    CtxBuf buf = { 0 };
    const char s[] = "abcd";

    _ctxBufInsert(&buf, (Utf8Ch *)s, chArrLen(s));

    _ctxBufRemove(&buf, 2);
    testAssert(buf.len == 2);
    testAssert(*_ctxBufGet(&buf, 0) == 'a');
    testAssert(*_ctxBufGet(&buf, 1) == 'b');

    memFree(buf.bytes);
}

void test_ctxBufRemoveFromMiddle(void) {
    CtxBuf buf = { 0 };
    const char s[] = "abcd";

    _ctxBufInsert(&buf, (Utf8Ch *)s, chArrLen(s));
    _ctxBufSetGapIdx(&buf, 2);
    _ctxBufRemove(&buf, 2);
    testAssert(buf.len == 2);
    testAssert(*_ctxBufGet(&buf, 0) == 'c');
    testAssert(*_ctxBufGet(&buf, 1) == 'd');

    memFree(buf.bytes);
}

testList(
    testMake(test_ctxBufRemoveEmpty),
    testMake(test_ctxBufRemoveFromEnd),
    testMake(test_ctxBufRemoveFromMiddle)
)
