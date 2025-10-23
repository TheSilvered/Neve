#include "nv_test.h"
#define lineRefMaxGap_ 4
#include "nv_context.c"

void test_ctxLineEndNoLineRef(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncd\ne";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    testAssert(ctxLineEnd_(&ctx, 0) == 2);
    testAssert(ctxLineEnd_(&ctx, 1) == 5);
    testAssert(ctxLineEnd_(&ctx, 2) == 7);
    testAssert(ctxLineEnd_(&ctx, 3) == -1);

    ctxDestroy(&ctx);
}

void test_ctxLineEndWithLineRef(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncd\ne";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    testAssert(ctxLineEnd_(&ctx, 0) == 2);
    testAssert(ctxLineEnd_(&ctx, 1) == 5);
    testAssert(ctxLineEnd_(&ctx, 2) == 9);
    testAssert(ctxLineEnd_(&ctx, 3) == 12);
    testAssert(ctxLineEnd_(&ctx, 4) == 14);
    testAssert(ctxLineEnd_(&ctx, 5) == -1);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxLineEndNoLineRef),
    testMake(test_ctxLineEndWithLineRef)
)

