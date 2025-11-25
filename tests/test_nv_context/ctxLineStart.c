#include "nv_test.h"
#define lineRefMaxGap_ 4
#include "nv_context.c"

void test_ctxLineStartNoLineRef(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncd\ne";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    testAssert(ctxLineStart_(&ctx, 0) == 0);
    testAssert(ctxLineStart_(&ctx, 1) == 3);
    testAssert(ctxLineStart_(&ctx, 2) == 6);
    testAssert(ctxLineStart_(&ctx, 3) == -1);

    ctxDestroy(&ctx);
}

void test_ctxLineStartWithLineRef(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncd\ne";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    testAssert(ctxLineStart_(&ctx, 0) == 0);
    testAssert(ctxLineStart_(&ctx, 1) == 3);
    testAssert(ctxLineStart_(&ctx, 2) == 6);
    testAssert(ctxLineStart_(&ctx, 3) == 10);
    testAssert(ctxLineStart_(&ctx, 4) == 13);
    testAssert(ctxLineStart_(&ctx, 5) == -1);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxLineStartNoLineRef),
    testMake(test_ctxLineStartWithLineRef)
)
