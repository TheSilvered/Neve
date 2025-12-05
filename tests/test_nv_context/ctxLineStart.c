#include "nv_test.h"
#define _lineRefMaxGap 4
#include "nv_context.c"

void test_ctxLineStartNoLineRef(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncd\ne";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    testAssert(_ctxLineStart(&ctx, 0) == 0);
    testAssert(_ctxLineStart(&ctx, 1) == 3);
    testAssert(_ctxLineStart(&ctx, 2) == 6);
    testAssert(_ctxLineStart(&ctx, 3) == -1);

    ctxDestroy(&ctx);
}

void test_ctxLineStartWithLineRef(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncd\ne";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    testAssert(_ctxLineStart(&ctx, 0) == 0);
    testAssert(_ctxLineStart(&ctx, 1) == 3);
    testAssert(_ctxLineStart(&ctx, 2) == 6);
    testAssert(_ctxLineStart(&ctx, 3) == 10);
    testAssert(_ctxLineStart(&ctx, 4) == 13);
    testAssert(_ctxLineStart(&ctx, 5) == -1);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxLineStartNoLineRef),
    testMake(test_ctxLineStartWithLineRef)
)
