#include "nv_test.h"
#define _lineRefMaxGap 4
#include "nv_context.c"

void test_ctxLineStartNoLineRef(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncd\ne";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    testAssert(ctxLineStart(&ctx, 0) == 0);
    testAssert(ctxLineStart(&ctx, 1) == 3);
    testAssert(ctxLineStart(&ctx, 2) == 6);
    testAssert(ctxLineStart(&ctx, 3) == -1);

    ctxDestroy(&ctx);
}

void test_ctxLineStartWithLineRef(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncd\ne";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    testAssert(ctxLineStart(&ctx, 0) == 0);
    testAssert(ctxLineStart(&ctx, 1) == 3);
    testAssert(ctxLineStart(&ctx, 2) == 6);
    testAssert(ctxLineStart(&ctx, 3) == 10);
    testAssert(ctxLineStart(&ctx, 4) == 13);
    testAssert(ctxLineStart(&ctx, 5) == -1);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxLineStartNoLineRef),
    testMake(test_ctxLineStartWithLineRef)
)
