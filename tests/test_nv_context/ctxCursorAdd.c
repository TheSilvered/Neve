#include "nv_test.h"
#include "nv_context.c"

void test_ctxCursorAdd(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcde";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 3);
    testAssert(ctx._cursors.len == 1);
    testAssert(ctx._cursors.items[0].idx = 3);

    ctxCurAdd(&ctx, 5);
    testAssert(ctx._cursors.len == 2);
    testAssert(ctx._cursors.items[0].idx = 3);
    testAssert(ctx._cursors.items[1].idx = 5);

    ctxCurAdd(&ctx, 2);
    testAssert(ctx._cursors.len == 3);
    testAssert(ctx._cursors.items[0].idx = 2);
    testAssert(ctx._cursors.items[1].idx = 3);
    testAssert(ctx._cursors.items[2].idx = 5);

    ctxCurAdd(&ctx, 4);
    testAssert(ctx._cursors.len == 4);
    testAssert(ctx._cursors.items[0].idx = 2);
    testAssert(ctx._cursors.items[1].idx = 3);
    testAssert(ctx._cursors.items[2].idx = 4);
    testAssert(ctx._cursors.items[3].idx = 5);

    ctxCurAdd(&ctx, 4);
    testAssert(ctx._cursors.len == 4);
    testAssert(ctx._cursors.items[0].idx = 2);
    testAssert(ctx._cursors.items[1].idx = 3);
    testAssert(ctx._cursors.items[2].idx = 4);
    testAssert(ctx._cursors.items[3].idx = 5);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxCursorAdd)
)

