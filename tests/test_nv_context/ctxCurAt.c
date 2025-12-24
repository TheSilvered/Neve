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

    testAssert(_ctxCurAt(&ctx, 0) == 0);
    testAssert(_ctxCurAt(&ctx, 1) == 0);
    testAssert(_ctxCurAt(&ctx, 2) == 0);
    testAssert(_ctxCurAt(&ctx, 3) == 1);
    testAssert(_ctxCurAt(&ctx, 4) == 1);
    testAssert(_ctxCurAt(&ctx, 5) == 2);
    testAssert(_ctxCurAt(&ctx, 6) == 2);
    testAssert(_ctxCurAt(&ctx, 7) == 2);
    testAssert(_ctxCurAt(&ctx, 8) == 2);
    testAssert(_ctxCurAt(&ctx, 9) == 3);
    testAssert(_ctxCurAt(&ctx, 10) == 3);
    testAssert(_ctxCurAt(&ctx, 11) == 3);
    testAssert(_ctxCurAt(&ctx, 12) == 3);
    testAssert(_ctxCurAt(&ctx, 13) == 3);
    testAssert(_ctxCurAt(&ctx, 14) == 3);
    testAssert(_ctxCurAt(&ctx, 15) == 3);
    testAssert(_ctxCurAt(&ctx, 16) == 4);
    testAssert(_ctxCurAt(&ctx, 17) == 5);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxCurAt)
)
