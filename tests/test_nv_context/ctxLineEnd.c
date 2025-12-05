#include "nv_test.h"
#define _lineRefMaxGap 4
#include "nv_context.c"

void test_ctxLineEndNoLineRef(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncd\ne";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    testAssert(_ctxLineEnd(&ctx, 0) == 2);
    testAssert(_ctxLineEnd(&ctx, 1) == 5);
    testAssert(_ctxLineEnd(&ctx, 2) == 7);
    testAssert(_ctxLineEnd(&ctx, 3) == -1);

    ctxDestroy(&ctx);
}

void test_ctxLineEndWithLineRef(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncd\ne";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    testAssert(_ctxLineEnd(&ctx, 0) == 2);
    testAssert(_ctxLineEnd(&ctx, 1) == 5);
    testAssert(_ctxLineEnd(&ctx, 2) == 9);
    testAssert(_ctxLineEnd(&ctx, 3) == 12);
    testAssert(_ctxLineEnd(&ctx, 4) == 14);
    testAssert(_ctxLineEnd(&ctx, 5) == -1);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxLineEndNoLineRef),
    testMake(test_ctxLineEndWithLineRef)
)
