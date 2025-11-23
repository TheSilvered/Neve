#include "nv_test.h"
#include "nv_context.c"

void test_ctxCurMoveToPrevWordEndAlpha(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abc \tdef\nghijkl";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 15);
    ctxCurMoveToPrevWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 8);
    ctxCurMoveToPrevWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 3);
    ctxCurMoveToPrevWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 0);
    ctxCurMoveToPrevWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 0);

    ctxDestroy(&ctx);
}

void test_ctxCurMoveToPrevWordEndAlnum(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "a9c \t123\n2hijk5";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 15);
    ctxCurMoveToPrevWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 8);
    ctxCurMoveToPrevWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 3);
    ctxCurMoveToPrevWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 0);
    ctxCurMoveToPrevWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 0);

    ctxDestroy(&ctx);
}

void test_ctxCurMoveToPrevWordEndPunctuation(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "()* \t.,-\n&%/!:;";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 15);
    ctxCurMoveToPrevWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 8);
    ctxCurMoveToPrevWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 3);
    ctxCurMoveToPrevWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 0);
    ctxCurMoveToPrevWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 0);

    ctxDestroy(&ctx);
}

void test_ctxCurMoveToPrevWordEndMixed(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abc9()*  ==?92\n\t& ab";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 20);
    ctxCurMoveToPrevWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 17);
    ctxCurMoveToPrevWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 14);
    ctxCurMoveToPrevWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 12);
    ctxCurMoveToPrevWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 7);
    ctxCurMoveToPrevWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 4);
    ctxCurMoveToPrevWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 0);
    ctxCurMoveToPrevWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 0);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxCurMoveToPrevWordEndAlpha),
    testMake(test_ctxCurMoveToPrevWordEndAlnum),
    testMake(test_ctxCurMoveToPrevWordEndPunctuation),
    testMake(test_ctxCurMoveToPrevWordEndMixed)
)
