#include "nv_test.h"
#include "nv_context.c"

void test_ctxCurMoveToTextStartWithCursors(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abc\ndef";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 0);
    ctxCurAdd(&ctx, 1);
    ctxCurAdd(&ctx, 2);
    ctxCurAdd(&ctx, 3);
    ctxCurAdd(&ctx, 4);
    ctxCurAdd(&ctx, 5);
    ctxCurAdd(&ctx, 7);

    ctxCurMoveToTextStart(&ctx);

    testAssert(ctx.cursors.len == 1);
    testAssert(ctx.cursors.items[0].idx == 0);
    testAssert(ctx.cursors.items[0].baseCol == 0);

    ctxDestroy(&ctx);
}

void test_ctxCurMoveToTextStartNoCursors(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abc\ndef";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurMoveToTextStart(&ctx);

    testAssert(ctx.cursors.len == 0);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxCurMoveToTextStartWithCursors),
    testMake(test_ctxCurMoveToTextStartNoCursors)
)
