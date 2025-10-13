#include "nv_test.h"
#include "nv_context.c"

void test_ctxCurAtIdx(void) {
    Ctx ctx;
    ctxInit(&ctx, true);

    ctxCursorAdd_(&ctx, 2);
    ctxCursorAdd_(&ctx, 4);
    ctxCursorAdd_(&ctx, 8);
    ctxCursorAdd_(&ctx, 15);
    ctxCursorAdd_(&ctx, 16);

    testAssert(ctxCursorAt_(&ctx, 0) == 0);
    testAssert(ctxCursorAt_(&ctx, 1) == 0);
    testAssert(ctxCursorAt_(&ctx, 2) == 0);
    testAssert(ctxCursorAt_(&ctx, 3) == 1);
    testAssert(ctxCursorAt_(&ctx, 4) == 1);
    testAssert(ctxCursorAt_(&ctx, 5) == 2);
    testAssert(ctxCursorAt_(&ctx, 6) == 2);
    testAssert(ctxCursorAt_(&ctx, 7) == 2);
    testAssert(ctxCursorAt_(&ctx, 8) == 2);
    testAssert(ctxCursorAt_(&ctx, 9) == 3);
    testAssert(ctxCursorAt_(&ctx, 10) == 3);
    testAssert(ctxCursorAt_(&ctx, 11) == 3);
    testAssert(ctxCursorAt_(&ctx, 12) == 3);
    testAssert(ctxCursorAt_(&ctx, 13) == 3);
    testAssert(ctxCursorAt_(&ctx, 14) == 3);
    testAssert(ctxCursorAt_(&ctx, 15) == 3);
    testAssert(ctxCursorAt_(&ctx, 16) == 4);
    testAssert(ctxCursorAt_(&ctx, 17) == 5);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxCurAtIdx)
)

