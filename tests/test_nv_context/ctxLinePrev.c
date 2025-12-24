#include "nv_test.h"
#include "nv_context.c"

void test_ctxLinePrevFirst(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncde\n\nef";

    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    UcdCP ch;
    ptrdiff_t i = ctxLinePrevStart(&ctx, 0, &ch);
    testAssertRequire(i == 1);
    testAssert(ch == 'b');

    i = ctxLinePrev(&ctx, i, &ch);
    testAssert(i == 0);
    testAssert(ch == 'a');

    i = ctxLinePrev(&ctx, i, &ch);
    testAssert(i == -1);
    testAssert(ch == -1);

    ctxDestroy(&ctx);
}

void test_ctxLinePrevMiddle(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncde\n\nef";

    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    UcdCP ch;
    ptrdiff_t i = ctxLinePrevStart(&ctx, 1, &ch);
    testAssertRequire(i == 5);
    testAssert(ch == 'e');

    i = ctxLinePrev(&ctx, i, &ch);
    testAssert(i == 4);
    testAssert(ch == 'd');

    i = ctxLinePrev(&ctx, i, &ch);
    testAssert(i == 3);
    testAssert(ch == 'c');

    i = ctxLinePrev(&ctx, i, &ch);
    testAssert(i == -1);
    testAssert(ch == -1);

    ctxDestroy(&ctx);
}

void test_ctxLinePrevEmpty(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncde\n\nef";

    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    UcdCP ch;
    ptrdiff_t i = ctxLinePrevStart(&ctx, 2, &ch);
    testAssert(i == -1);
    testAssert(ch == -1);

    ctxDestroy(&ctx);
}

void test_ctxLinePrevLast(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncde\n\nef";

    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    UcdCP ch;
    ptrdiff_t i = ctxLinePrevStart(&ctx, 3, &ch);
    testAssert(i == 9);
    testAssert(ch == 'f');

    i = ctxLinePrev(&ctx, i, &ch);
    testAssertRequire(i == 8);
    testAssert(ch == 'e');

    i = ctxLinePrev(&ctx, i, &ch);
    testAssert(i == -1);
    testAssert(ch == -1);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxLinePrevFirst),
    testMake(test_ctxLinePrevMiddle),
    testMake(test_ctxLinePrevLast),
    testMake(test_ctxLinePrevEmpty)
)
