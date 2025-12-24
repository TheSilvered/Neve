#include "nv_test.h"
#include "nv_context.c"

void test_ctxCurMoveToNextWordEndAlpha(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abc \tdef\nghijkl";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    ctxCurAdd(&ctx, 0);
    ctxCurMoveToNextWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 3);
    ctxCurMoveToNextWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 8);
    ctxCurMoveToNextWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 15);
    ctxCurMoveToNextWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 15);

    ctxDestroy(&ctx);
}

void test_ctxCurMoveToNextWordEndAlnum(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "a9c \t123\n2hijk5";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    ctxCurAdd(&ctx, 0);
    ctxCurMoveToNextWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 3);
    ctxCurMoveToNextWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 8);
    ctxCurMoveToNextWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 15);
    ctxCurMoveToNextWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 15);

    ctxDestroy(&ctx);
}

void test_ctxCurMoveToNextWordEndPunctuation(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "()* \t.,-\n&%/!:;";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    ctxCurAdd(&ctx, 0);
    ctxCurMoveToNextWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 3);
    ctxCurMoveToNextWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 8);
    ctxCurMoveToNextWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 15);
    ctxCurMoveToNextWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 15);

    ctxDestroy(&ctx);
}

void test_ctxCurMoveToNextWordEndMixed(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abc9()*  ==?92\n\t& ab";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    ctxCurAdd(&ctx, 0);
    ctxCurMoveToNextWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 4);
    ctxCurMoveToNextWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 7);
    ctxCurMoveToNextWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 12);
    ctxCurMoveToNextWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 14);
    ctxCurMoveToNextWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 17);
    ctxCurMoveToNextWordEnd(&ctx);
    testAssert(ctx.cursors.items[0].idx == 20);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxCurMoveToNextWordEndAlpha),
    testMake(test_ctxCurMoveToNextWordEndAlnum),
    testMake(test_ctxCurMoveToNextWordEndPunctuation),
    testMake(test_ctxCurMoveToNextWordEndMixed)
)
