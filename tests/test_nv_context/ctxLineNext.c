#include "nv_test.h"
#include "nv_context.c"

void test_ctxLineNextFirst(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncde\n\nef";

    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    UcdCP ch;
    ptrdiff_t i = ctxLineNextStart(&ctx, 0, &ch);
    testAssertRequire(i == 0);
    testAssert(ch == 'a');

    i = ctxLineNext(&ctx, i, &ch);
    testAssert(i == 1);
    testAssert(ch == 'b');

    i = ctxLineNext(&ctx, i, &ch);
    testAssert(i == -1);
    testAssert(ch == -1);

    ctxDestroy(&ctx);
}

void test_ctxLineNextMiddle(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncde\n\nef";

    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    UcdCP ch;
    ptrdiff_t i = ctxLineNextStart(&ctx, 1, &ch);
    testAssertRequire(i == 3);
    testAssert(ch == 'c');

    i = ctxLineNext(&ctx, i, &ch);
    testAssert(i == 4);
    testAssert(ch == 'd');

    i = ctxLineNext(&ctx, i, &ch);
    testAssert(i == 5);
    testAssert(ch == 'e');

    i = ctxLineNext(&ctx, i, &ch);
    testAssert(i == -1);
    testAssert(ch == -1);

    ctxDestroy(&ctx);
}

void test_ctxLineNextEmpty(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncde\n\nef";

    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    UcdCP ch;
    ptrdiff_t i = ctxLineNextStart(&ctx, 2, &ch);
    testAssert(i == -1);
    testAssert(ch == -1);

    ctxDestroy(&ctx);
}

void test_ctxLineNextLast(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncde\n\nef";

    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    UcdCP ch;
    ptrdiff_t i = ctxLineNextStart(&ctx, 3, &ch);
    testAssertRequire(i == 8);
    testAssert(ch == 'e');

    i = ctxLineNext(&ctx, i, &ch);
    testAssert(i == 9);
    testAssert(ch == 'f');

    i = ctxLineNext(&ctx, i, &ch);
    testAssert(i == -1);
    testAssert(ch == -1);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxLineNextFirst),
    testMake(test_ctxLineNextMiddle),
    testMake(test_ctxLineNextLast),
    testMake(test_ctxLineNextEmpty)
)

