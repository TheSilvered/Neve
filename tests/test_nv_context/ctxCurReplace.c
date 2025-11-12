#include "nv_test.h"
#include "nv_context.c"

void test_ctxCurReplace(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    ctxInit(&ctx, true);
    const char s[] = "abcdefgh";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 2);
    ctxCurAdd(&ctx, 4);
    ctxCurAdd(&ctx, 6);

    ctxCurReplace(&ctx, 6, 7);
    testAssert(ctx.cursors.len == 3);
    testAssert(ctx.cursors.items[0].idx == 2);
    testAssert(ctx.cursors.items[1].idx == 4);
    testAssert(ctx.cursors.items[2].idx == 7);

    ctxCurReplace(&ctx, 2, 5);
    testAssert(ctx.cursors.len == 3);
    testAssert(ctx.cursors.items[0].idx == 4);
    testAssert(ctx.cursors.items[1].idx == 5);
    testAssert(ctx.cursors.items[2].idx == 7);

    ctxCurReplace(&ctx, 5, 6);
    testAssert(ctx.cursors.len == 3);
    testAssert(ctx.cursors.items[0].idx == 4);
    testAssert(ctx.cursors.items[1].idx == 6);
    testAssert(ctx.cursors.items[2].idx == 7);

    ctxCurReplace(&ctx, 1, 8);
    testAssert(ctx.cursors.len == 4);
    testAssert(ctx.cursors.items[0].idx == 4);
    testAssert(ctx.cursors.items[1].idx == 6);
    testAssert(ctx.cursors.items[2].idx == 7);
    testAssert(ctx.cursors.items[3].idx == 8);

    ctxCurReplace(&ctx, 6, 7);
    testAssert(ctx.cursors.len == 3);
    testAssert(ctx.cursors.items[0].idx == 4);
    testAssert(ctx.cursors.items[1].idx == 7);
    testAssert(ctx.cursors.items[2].idx == 8);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxCurReplace)
)

