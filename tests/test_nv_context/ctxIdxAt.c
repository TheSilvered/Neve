#include "nv_test.h"
#define _lineRefMaxGap 4
#include "nv_context.c"

void test_ctxIdxAtNoLineRef(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "a\n\td";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    size_t col;
    testAssert(ctxIdxAt(&ctx, 0, 0, &col) == 0);
    testAssert(col == 0);
    testAssert(ctxIdxAt(&ctx, 0, 1, &col) == 1);
    testAssert(col == 1);
    testAssert(ctxIdxAt(&ctx, 0, 2, &col) == 1);
    testAssert(col == 1);
    testAssert(ctxIdxAt(&ctx, 0, 9, &col) == 1);
    testAssert(col == 1);
    testAssert(ctxIdxAt(&ctx, 1, 0, &col) == 2);
    testAssert(col == 0);
    testAssert(ctxIdxAt(&ctx, 1, 1, &col) == 2);
    testAssert(col == 0);
    testAssert(ctxIdxAt(&ctx, 1, 4, &col) == 2);
    testAssert(col == 0);
    testAssert(ctxIdxAt(&ctx, 1, 5, &col) == 3);
    testAssert(col == 8);
    testAssert(ctxIdxAt(&ctx, 1, 7, &col) == 3);
    testAssert(col == 8);
    testAssert(ctxIdxAt(&ctx, 1, 8, &col) == 3);
    testAssert(col == 8);
    testAssert(ctxIdxAt(&ctx, 1, 9, &col) == 4);
    testAssert(col == 9);

    ctxDestroy(&ctx);
}

void test_ctxIdxAtWithLineRef(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    // s = "aè😊\n\td😊"
    const char s[] = "a\xc3\xa8\xf0\x9f\x98\x8a\n\td\xf0\x9f\x98\x8a";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    size_t col;
    testAssert(ctxIdxAt(&ctx, 0, 0, &col) == 0);
    testAssert(col == 0);
    testAssert(ctxIdxAt(&ctx, 0, 1, &col) == 1);
    testAssert(col == 1);
    testAssert(ctxIdxAt(&ctx, 0, 2, &col) == 3);
    testAssert(col == 2);
    testAssert(ctxIdxAt(&ctx, 0, 3, &col) == 3);
    testAssert(col == 2);
    testAssert(ctxIdxAt(&ctx, 0, 4, &col) == 7);
    testAssert(col == 4);
    testAssert(ctxIdxAt(&ctx, 0, 5, &col) == 7);
    testAssert(col == 4);
    testAssert(ctxIdxAt(&ctx, 0, 9, &col) == 7);
    testAssert(col == 4);
    testAssert(ctxIdxAt(&ctx, 1, 0, &col) == 8);
    testAssert(col == 0);
    testAssert(ctxIdxAt(&ctx, 1, 1, &col) == 8);
    testAssert(col == 0);
    testAssert(ctxIdxAt(&ctx, 1, 4, &col) == 8);
    testAssert(col == 0);
    testAssert(ctxIdxAt(&ctx, 1, 5, &col) == 9);
    testAssert(col == 8);
    testAssert(ctxIdxAt(&ctx, 1, 7, &col) == 9);
    testAssert(col == 8);
    testAssert(ctxIdxAt(&ctx, 1, 8, &col) == 9);
    testAssert(col == 8);
    testAssert(ctxIdxAt(&ctx, 1, 9, &col) == 10);
    testAssert(col == 9);
    testAssert(ctxIdxAt(&ctx, 1, 10, &col) == 10);
    testAssert(col == 9);
    testAssert(ctxIdxAt(&ctx, 1, 11, &col) == 14);
    testAssert(col == 11);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxIdxAtNoLineRef),
    testMake(test_ctxIdxAtWithLineRef)
)
