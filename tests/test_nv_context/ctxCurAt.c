#include "nv_test.h"
#include "nv_context.c"

void test_ctxCurAt(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefghijklmnopq";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    ctxCurAdd(&ctx, 2);
    ctxCurAdd(&ctx, 4);
    ctxCurAdd(&ctx, 8);
    ctxCurAdd(&ctx, 15);
    ctxCurAdd(&ctx, 16);

    testAssert(_ctxCurAt(&ctx.cursors, 0) == 0);
    testAssert(_ctxCurAt(&ctx.cursors, 1) == 0);
    testAssert(_ctxCurAt(&ctx.cursors, 2) == 0);
    testAssert(_ctxCurAt(&ctx.cursors, 3) == 1);
    testAssert(_ctxCurAt(&ctx.cursors, 4) == 1);
    testAssert(_ctxCurAt(&ctx.cursors, 5) == 2);
    testAssert(_ctxCurAt(&ctx.cursors, 6) == 2);
    testAssert(_ctxCurAt(&ctx.cursors, 7) == 2);
    testAssert(_ctxCurAt(&ctx.cursors, 8) == 2);
    testAssert(_ctxCurAt(&ctx.cursors, 9) == 3);
    testAssert(_ctxCurAt(&ctx.cursors, 10) == 3);
    testAssert(_ctxCurAt(&ctx.cursors, 11) == 3);
    testAssert(_ctxCurAt(&ctx.cursors, 12) == 3);
    testAssert(_ctxCurAt(&ctx.cursors, 13) == 3);
    testAssert(_ctxCurAt(&ctx.cursors, 14) == 3);
    testAssert(_ctxCurAt(&ctx.cursors, 15) == 3);
    testAssert(_ctxCurAt(&ctx.cursors, 16) == 4);
    testAssert(_ctxCurAt(&ctx.cursors, 17) == 5);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxCurAt)
)
