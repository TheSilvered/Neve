#include "nv_test.h"
#include "nv_context.c"

void test_ctxCurMoveLineStartSingleLine(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcd";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 0);
    ctxCurAdd(&ctx, 1);
    ctxCurAdd(&ctx, 2);
    ctxCurAdd(&ctx, 3);
    ctxCurAdd(&ctx, 4);

    ctxCurMoveToLineStart(&ctx);

    testAssert(ctx.cursors.len == 1);
    testAssert(ctx.cursors.items[0].idx == 0);
    testAssert(ctx.cursors.items[0].baseCol == 0);

    ctxDestroy(&ctx);
}

void test_ctxCurMoveLineStartMultiline(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncde\nfg";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 0);
    ctxCurAdd(&ctx, 1);
    ctxCurAdd(&ctx, 3);
    ctxCurAdd(&ctx, 6);
    ctxCurAdd(&ctx, 8);
    ctxCurAdd(&ctx, 9);

    ctxCurMoveToLineStart(&ctx);

    testAssert(ctx.cursors.len == 3);
    testAssert(ctx.cursors.items[0].idx == 0);
    testAssert(ctx.cursors.items[0].baseCol == 0);
    testAssert(ctx.cursors.items[1].idx == 3);
    testAssert(ctx.cursors.items[1].baseCol == 0);
    testAssert(ctx.cursors.items[2].idx == 7);
    testAssert(ctx.cursors.items[2].baseCol == 0);
}

testList(
    testMake(test_ctxCurMoveLineStartSingleLine),
    testMake(test_ctxCurMoveLineStartMultiline)
)
