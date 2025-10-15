#include "nv_test.h"
#define lineRefMaxGap_ 4
#include "nv_context.c"

void test_ctxLineNextStartEmpty(void) {
    Ctx ctx;
    ctxInit(&ctx, true);

    ptrdiff_t i = ctxLineNextStart(&ctx, 0, NULL);
    testAssert(i == -1);
    i = ctxLineNextStart(&ctx, 1, NULL);
    testAssert(i == -1);

    ctxDestroy(&ctx);
}

void test_ctxLineNextStartOneLine(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    UcdCP cp = 0;
    ptrdiff_t i = ctxLineNextStart(&ctx, 0, &cp);
    testAssert(i == 0);
    testAssert(cp == 'a');

    i = ctxLineNextStart(&ctx, 1, &cp);
    testAssert(i == -1);
    testAssert(cp == -1);

    ctxDestroy(&ctx);
}

void test_ctxLineNextStartMultiLine(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncde\n\nfg";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    UcdCP cp = 0;
    ptrdiff_t i = ctxLineNextStart(&ctx, 0, &cp);
    testAssert(i == 0);
    testAssert(cp == 'a');

    i = ctxLineNextStart(&ctx, 1, &cp);
    testAssert(i == 3);
    testAssert(cp == 'c');

    i = ctxLineNextStart(&ctx, 2, &cp);
    testAssert(i == -1);
    testAssert(cp == -1);

    i = ctxLineNextStart(&ctx, 3, &cp);
    testAssert(i == 8);
    testAssert(cp == 'f');

    i = ctxLineNextStart(&ctx, 4, &cp);
    testAssert(i == -1);
    testAssert(cp == -1);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxLineNextStartEmpty),
    testMake(test_ctxLineNextStartOneLine),
    testMake(test_ctxLineNextStartMultiLine)
)

