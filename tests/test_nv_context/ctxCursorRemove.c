#include "nv_test.h"
#include "nv_context.c"

void test_ctxCursorRemove(void) {
    Ctx ctx;
    ctxInit(&ctx, true);

    ctxCurAdd(&ctx, 2);
    ctxCurAdd(&ctx, 4);
    ctxCurAdd(&ctx, 8);
    ctxCurAdd(&ctx, 15);
    ctxCurAdd(&ctx, 16);

    ctxCurRemove(&ctx, 3);
    testAssert(ctx._cursors.len == 5);
    testAssert(ctx._cursors.items[0].idx == 2);
    testAssert(ctx._cursors.items[1].idx == 4);
    testAssert(ctx._cursors.items[2].idx == 8);
    testAssert(ctx._cursors.items[3].idx == 15);
    testAssert(ctx._cursors.items[4].idx == 16);

    ctxCurRemove(&ctx, 2);
    testAssert(ctx._cursors.len == 4);
    testAssert(ctx._cursors.items[0].idx == 4);
    testAssert(ctx._cursors.items[1].idx == 8);
    testAssert(ctx._cursors.items[2].idx == 15);
    testAssert(ctx._cursors.items[3].idx == 16);

    ctxCurRemove(&ctx, 8);
    testAssert(ctx._cursors.len == 3);
    testAssert(ctx._cursors.items[0].idx == 4);
    testAssert(ctx._cursors.items[1].idx == 15);
    testAssert(ctx._cursors.items[2].idx == 16);

    ctxCurRemove(&ctx, 16);
    testAssert(ctx._cursors.len == 2);
    testAssert(ctx._cursors.items[0].idx == 4);
    testAssert(ctx._cursors.items[1].idx == 15);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxCursorRemove)
)

