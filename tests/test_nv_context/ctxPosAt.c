#include "nv_test.h"
#define lineRefMaxGap_ 4
#include "nv_context.c"

void test_ctxPosAtNoLineRef(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\nc";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    size_t line, col;
    ctxPosAt_(&ctx, 0, &line, &col);

    testAssert(line == 0);
    testAssert(col == 0);

    ctxPosAt_(&ctx, 1, &line, &col);

    testAssert(line == 0);
    testAssert(col == 1);

    ctxPosAt_(&ctx, 2, &line, &col);

    testAssert(line == 0);
    testAssert(col == 2);

    ctxPosAt_(&ctx, 3, &line, &col);

    testAssert(line == 1);
    testAssert(col == 0);

    ctxPosAt_(&ctx, 4, &line, &col);

    testAssert(line == 1);
    testAssert(col == 1);

    ctxDestroy(&ctx);
}

void test_ctxPosAtWithLineRef(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncd\ne";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    size_t line, col;
    ctxPosAt_(&ctx, 0, &line, &col);

    testAssert(line == 0);
    testAssert(col == 0);

    ctxPosAt_(&ctx, 2, &line, &col);

    testAssert(line == 0);
    testAssert(col == 2);

    ctxPosAt_(&ctx, 3, &line, &col);

    testAssert(line == 1);
    testAssert(col == 0);

    ctxPosAt_(&ctx, 7, &line, &col);

    testAssert(line == 2);
    testAssert(col == 1);

    ctxPosAt_(&ctx, 8, &line, &col);

    testAssert(line == 2);
    testAssert(col == 2);

    ctxPosAt_(&ctx, 9, &line, &col);

    testAssert(line == 2);
    testAssert(col == 3);

    ctxPosAt_(&ctx, 10, &line, &col);

    testAssert(line == 3);
    testAssert(col == 0);

    ctxPosAt_(&ctx, 11, &line, &col);

    testAssert(line == 3);
    testAssert(col == 1);

    ctxPosAt_(&ctx, 12, &line, &col);

    testAssert(line == 3);
    testAssert(col == 2);

    ctxPosAt_(&ctx, 13, &line, &col);

    testAssert(line == 4);
    testAssert(col == 0);

    ctxPosAt_(&ctx, 14, &line, &col);

    testAssert(line == 4);
    testAssert(col == 1);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxPosAtNoLineRef),
    testMake(test_ctxPosAtWithLineRef)
)
