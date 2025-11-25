#include "nv_test.h"
#include "nv_context.c"

void test_ctxCurMoveLineEndSingleLine(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcd";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 0);
    ctxCurAdd(&ctx, 1);
    ctxCurAdd(&ctx, 2);
    ctxCurAdd(&ctx, 3);
    ctxCurAdd(&ctx, 4);

    ctxCurMoveToLineEnd(&ctx);

    testAssert(ctx.cursors.len == 1);
    testAssert(ctx.cursors.items[0].idx == 4);
    testAssert(ctx.cursors.items[0].baseCol == 4);

    ctxDestroy(&ctx);
}

void test_ctxCurMoveLineEndMultiline(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncde\nfg";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 0);
    ctxCurAdd(&ctx, 2);
    ctxCurAdd(&ctx, 3);
    ctxCurAdd(&ctx, 6);
    ctxCurAdd(&ctx, 8);
    ctxCurAdd(&ctx, 9);

    ctxCurMoveToLineEnd(&ctx);

    testAssert(ctx.cursors.len == 3);
    testAssert(ctx.cursors.items[0].idx == 2);
    testAssert(ctx.cursors.items[0].baseCol == 2);
    testAssert(ctx.cursors.items[1].idx == 6);
    testAssert(ctx.cursors.items[1].baseCol == 3);
    testAssert(ctx.cursors.items[2].idx == 9);
    testAssert(ctx.cursors.items[2].baseCol == 2);
}

testList(
    testMake(test_ctxCurMoveLineEndSingleLine),
    testMake(test_ctxCurMoveLineEndMultiline)
)
