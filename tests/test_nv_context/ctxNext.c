#include "nv_test.h"
#include "nv_context.c"

void test_ctxNextEmpty(void) {
    Ctx ctx;
    ctxInit(&ctx, true);

    UcdCP cp;
    ptrdiff_t i = ctxNext(&ctx, -1, &cp);
    testAssert(i == -1);
    testAssert(cp == -1);

    ctxDestroy(&ctx);
}

void test_ctxNextAscii(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcd";

    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    UcdCP cp;
    ptrdiff_t i = ctxNext(&ctx, -1, &cp);
    testAssert(i == 0);
    testAssert(cp == 'a');
    i = ctxNext(&ctx, i, &cp);
    testAssert(i == 1);
    testAssert(cp == 'b');
    i = ctxNext(&ctx, i, &cp);
    testAssert(i == 2);
    testAssert(cp == 'c');
    i = ctxNext(&ctx, i, &cp);
    testAssert(i == 3);
    testAssert(cp == 'd');
    i = ctxNext(&ctx, i, &cp);
    testAssert(i == -1);
    testAssert(cp == -1);

    ctxDestroy(&ctx);
}

void test_ctxNextUTF8(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "a\xc3\xa8\xf0\x9f\x98\x8a\xe4\xb8\x96\xe7\x95\x8c";

    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    UcdCP cp;
    ptrdiff_t i = ctxNext(&ctx, -1, &cp);
    testAssert(i == 0);
    testAssert(cp == 'a');
    i = ctxNext(&ctx, i, &cp);
    testAssert(i == 1);
    testAssert(cp == 0xe8);
    i = ctxNext(&ctx, i, &cp);
    testAssert(i == 3);
    testAssert(cp == 0x1f60a);
    i = ctxNext(&ctx, i, &cp);
    testAssert(i == 7);
    testAssert(cp == 0x4e16);
    i = ctxNext(&ctx, i, &cp);
    testAssert(i == 10);
    testAssert(cp == 0x754c);
    i = ctxNext(&ctx, i, &cp);
    testAssert(i == -1);
    testAssert(cp == -1);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxNextEmpty),
    testMake(test_ctxNextAscii),
    testMake(test_ctxNextUTF8)
)
