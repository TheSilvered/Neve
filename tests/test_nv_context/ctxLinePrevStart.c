#include "nv_test.h"
#define _lineRefMaxGap 4
#include "nv_context.c"

void test_ctxLinePrevStartEmpty(void) {
    Ctx ctx;
    ctxInit(&ctx, true);

    ptrdiff_t i = ctxLinePrevStart(&ctx, 0, NULL);
    testAssert(i == -1);
    i = ctxLinePrevStart(&ctx, 1, NULL);
    testAssert(i == -1);

    ctxDestroy(&ctx);
}

void test_ctxLinePrevStartOneLine(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    UcdCP cp = 0;
    ptrdiff_t i = ctxLinePrevStart(&ctx, 0, &cp);
    testAssert(i == 6);
    testAssert(cp == 'g');

    i = ctxLinePrevStart(&ctx, 1, &cp);
    testAssert(i == -1);
    testAssert(cp == -1);

    ctxDestroy(&ctx);
}

void test_ctxLinePrevStartMultiLine(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncde\n\nfg";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    UcdCP cp = 0;
    ptrdiff_t i = ctxLinePrevStart(&ctx, 0, &cp);
    testAssert(i == 1);
    testAssert(cp == 'b');

    i = ctxLinePrevStart(&ctx, 1, &cp);
    testAssert(i == 5);
    testAssert(cp == 'e');

    i = ctxLinePrevStart(&ctx, 2, &cp);
    testAssert(i == -1);
    testAssert(cp == -1);

    i = ctxLinePrevStart(&ctx, 3, &cp);
    testAssert(i == 9);
    testAssert(cp == 'g');

    i = ctxLinePrevStart(&ctx, 4, &cp);
    testAssert(i == -1);
    testAssert(cp == -1);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxLinePrevStartEmpty),
    testMake(test_ctxLinePrevStartOneLine),
    testMake(test_ctxLinePrevStartMultiLine)
)
