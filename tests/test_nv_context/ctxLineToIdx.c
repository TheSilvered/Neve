#include "nv_test.h"
#define lineRefMaxGap_ 4
#include "nv_context.c"

void test_ctxLineToIdxNoLineRef(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncd\ne";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    testAssert(ctxLineToIdx_(&ctx, 0) == 0);
    testAssert(ctxLineToIdx_(&ctx, 1) == 3);
    testAssert(ctxLineToIdx_(&ctx, 2) == 6);
    testAssert(ctxLineToIdx_(&ctx, 3) == -1);

    ctxDestroy(&ctx);
}

void test_ctxLineToIdxWithLineRef(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncd\ne";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    testAssert(ctxLineToIdx_(&ctx, 0) == 0);
    testAssert(ctxLineToIdx_(&ctx, 1) == 3);
    testAssert(ctxLineToIdx_(&ctx, 2) == 6);
    testAssert(ctxLineToIdx_(&ctx, 3) == 10);
    testAssert(ctxLineToIdx_(&ctx, 4) == 13);
    testAssert(ctxLineToIdx_(&ctx, 5) == -1);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxLineToIdxNoLineRef),
    testMake(test_ctxLineToIdxWithLineRef)
)

