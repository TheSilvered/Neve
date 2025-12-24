#include "nv_test.h"
#include "nv_context.c"

void test_ctxCurRemove(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    ctxInit(&ctx, true);
    const char s[] = "abcdefghijklmnop";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    ctxCurAdd(&ctx, 2);
    ctxCurAdd(&ctx, 4);
    ctxCurAdd(&ctx, 8);
    ctxCurAdd(&ctx, 15);
    ctxCurAdd(&ctx, 16);

    ctxCurRemove(&ctx, 3);
    testAssert(ctx.cursors.len == 5);
    testAssert(ctx.cursors.items[0].idx == 2);
    testAssert(ctx.cursors.items[1].idx == 4);
    testAssert(ctx.cursors.items[2].idx == 8);
    testAssert(ctx.cursors.items[3].idx == 15);
    testAssert(ctx.cursors.items[4].idx == 16);

    ctxCurRemove(&ctx, 2);
    testAssert(ctx.cursors.len == 4);
    testAssert(ctx.cursors.items[0].idx == 4);
    testAssert(ctx.cursors.items[1].idx == 8);
    testAssert(ctx.cursors.items[2].idx == 15);
    testAssert(ctx.cursors.items[3].idx == 16);

    ctxCurRemove(&ctx, 8);
    testAssert(ctx.cursors.len == 3);
    testAssert(ctx.cursors.items[0].idx == 4);
    testAssert(ctx.cursors.items[1].idx == 15);
    testAssert(ctx.cursors.items[2].idx == 16);

    ctxCurRemove(&ctx, 16);
    testAssert(ctx.cursors.len == 2);
    testAssert(ctx.cursors.items[0].idx == 4);
    testAssert(ctx.cursors.items[1].idx == 15);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxCurRemove)
)
