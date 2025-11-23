#include "nv_test.h"
#include "nv_context.c"

void test_ctxPrevEmpty(void) {
    Ctx ctx;
    ctxInit(&ctx, true);

    UcdCP cp;
    ptrdiff_t i = ctxPrev(&ctx, -1, &cp);
    testAssert(i == -1);
    testAssert(cp == -1);

    ctxDestroy(&ctx);
}

void test_ctxPrevAscii(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcd";

    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    UcdCP cp;
    ptrdiff_t i = ctxPrev(&ctx, -1, &cp);
    testAssert(i == 3);
    testAssert(cp == 'd');
    i = ctxPrev(&ctx, i, &cp);
    testAssert(i == 2);
    testAssert(cp == 'c');
    i = ctxPrev(&ctx, i, &cp);
    testAssert(i == 1);
    testAssert(cp == 'b');
    i = ctxPrev(&ctx, i, &cp);
    testAssert(i == 0);
    testAssert(cp == 'a');
    i = ctxPrev(&ctx, i, &cp);
    testAssert(i == -1);
    testAssert(cp == -1);

    ctxDestroy(&ctx);
}

void test_ctxPrevUTF8(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "a\xc3\xa8\xf0\x9f\x98\x8a\xe4\xb8\x96\xe7\x95\x8c";

    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    UcdCP cp;
    ptrdiff_t i = ctxPrev(&ctx, -1, &cp);
    testAssert(i == 10);
    testAssert(cp == 0x754c);
    i = ctxPrev(&ctx, i, &cp);
    testAssert(i == 7);
    testAssert(cp == 0x4e16);
    i = ctxPrev(&ctx, i, &cp);
    testAssert(i == 3);
    testAssert(cp == 0x1f60a);
    i = ctxPrev(&ctx, i, &cp);
    testAssert(i == 1);
    testAssert(cp == 0xe8);
    i = ctxPrev(&ctx, i, &cp);
    testAssert(i == 0);
    testAssert(cp == 'a');
    i = ctxPrev(&ctx, i, &cp);
    testAssert(i == -1);
    testAssert(cp == -1);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxPrevEmpty),
    testMake(test_ctxPrevAscii),
    testMake(test_ctxPrevUTF8)
)

