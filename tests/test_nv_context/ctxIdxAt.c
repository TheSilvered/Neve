#include "nv_test.h"
#define lineRefMaxGap_ 4
#include "nv_context.c"

void test_ctxIdxAtNoLineRef(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "a\n\td";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    testAssert(ctxIdxAt_(&ctx, 0, 0) == 0);
    testAssert(ctxIdxAt_(&ctx, 0, 1) == 1);
    testAssert(ctxIdxAt_(&ctx, 0, 2) == 1);
    testAssert(ctxIdxAt_(&ctx, 0, 9) == 1);
    testAssert(ctxIdxAt_(&ctx, 1, 0) == 2);
    testAssert(ctxIdxAt_(&ctx, 1, 1) == 2);
    testAssert(ctxIdxAt_(&ctx, 1, 4) == 2);
    testAssert(ctxIdxAt_(&ctx, 1, 5) == 3);
    testAssert(ctxIdxAt_(&ctx, 1, 7) == 3);
    testAssert(ctxIdxAt_(&ctx, 1, 8) == 3);
    testAssert(ctxIdxAt_(&ctx, 1, 9) == 4);

    ctxDestroy(&ctx);
}

void test_ctxIdxAtWithLineRef(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    // s = "aè😊\n\td😊"
    const char s[] = "a\xc3\xa8\xf0\x9f\x98\x8a\n\td\xf0\x9f\x98\x8a";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    testAssert(ctxIdxAt_(&ctx, 0, 0) == 0);
    testAssert(ctxIdxAt_(&ctx, 0, 1) == 1);
    testAssert(ctxIdxAt_(&ctx, 0, 2) == 3);
    testAssert(ctxIdxAt_(&ctx, 0, 3) == 3);
    testAssert(ctxIdxAt_(&ctx, 0, 4) == 7);
    testAssert(ctxIdxAt_(&ctx, 0, 5) == 7);
    testAssert(ctxIdxAt_(&ctx, 0, 9) == 7);
    testAssert(ctxIdxAt_(&ctx, 1, 0) == 8);
    testAssert(ctxIdxAt_(&ctx, 1, 1) == 8);
    testAssert(ctxIdxAt_(&ctx, 1, 4) == 8);
    testAssert(ctxIdxAt_(&ctx, 1, 5) == 9);
    testAssert(ctxIdxAt_(&ctx, 1, 7) == 9);
    testAssert(ctxIdxAt_(&ctx, 1, 8) == 9);
    testAssert(ctxIdxAt_(&ctx, 1, 9) == 10);
    testAssert(ctxIdxAt_(&ctx, 1, 10) == 10);
    testAssert(ctxIdxAt_(&ctx, 1, 11) == 14);
}

testList(
    testMake(test_ctxIdxAtNoLineRef),
    testMake(test_ctxIdxAtWithLineRef)
)
