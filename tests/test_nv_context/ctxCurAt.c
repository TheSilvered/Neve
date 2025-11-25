#include "nv_test.h"
#include "nv_context.c"

void test_ctxCurAt(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefghijklmnopq";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 2);
    ctxCurAdd(&ctx, 4);
    ctxCurAdd(&ctx, 8);
    ctxCurAdd(&ctx, 15);
    ctxCurAdd(&ctx, 16);

    testAssert(ctxCurAt_(&ctx, 0) == 0);
    testAssert(ctxCurAt_(&ctx, 1) == 0);
    testAssert(ctxCurAt_(&ctx, 2) == 0);
    testAssert(ctxCurAt_(&ctx, 3) == 1);
    testAssert(ctxCurAt_(&ctx, 4) == 1);
    testAssert(ctxCurAt_(&ctx, 5) == 2);
    testAssert(ctxCurAt_(&ctx, 6) == 2);
    testAssert(ctxCurAt_(&ctx, 7) == 2);
    testAssert(ctxCurAt_(&ctx, 8) == 2);
    testAssert(ctxCurAt_(&ctx, 9) == 3);
    testAssert(ctxCurAt_(&ctx, 10) == 3);
    testAssert(ctxCurAt_(&ctx, 11) == 3);
    testAssert(ctxCurAt_(&ctx, 12) == 3);
    testAssert(ctxCurAt_(&ctx, 13) == 3);
    testAssert(ctxCurAt_(&ctx, 14) == 3);
    testAssert(ctxCurAt_(&ctx, 15) == 3);
    testAssert(ctxCurAt_(&ctx, 16) == 4);
    testAssert(ctxCurAt_(&ctx, 17) == 5);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxCurAt)
)
