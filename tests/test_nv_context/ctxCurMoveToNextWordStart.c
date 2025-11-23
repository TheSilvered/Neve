#include "nv_test.h"
#include "nv_context.c"

void test_ctxCurMoveToNextWordStartAlpha(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abc \tdef\nghijkl";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 0);
    ctxCurMoveToNextWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 5);
    ctxCurMoveToNextWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 9);
    ctxCurMoveToNextWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 15);
    ctxCurMoveToNextWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 15);

    ctxDestroy(&ctx);
}

void test_ctxCurMoveToNextWordStartAlnum(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "a9c \t123\n2hijk5";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 0);
    ctxCurMoveToNextWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 5);
    ctxCurMoveToNextWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 9);
    ctxCurMoveToNextWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 15);
    ctxCurMoveToNextWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 15);

    ctxDestroy(&ctx);
}

void test_ctxCurMoveToNextWordStartPunctuation(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "()* \t.,-\n&%/!:;";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 0);
    ctxCurMoveToNextWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 5);
    ctxCurMoveToNextWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 9);
    ctxCurMoveToNextWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 15);
    ctxCurMoveToNextWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 15);

    ctxDestroy(&ctx);
}

void test_ctxCurMoveToNextWordStartMixed(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abc9()*  ==?92\n\t& ab";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 0);
    ctxCurMoveToNextWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 4);
    ctxCurMoveToNextWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 9);
    ctxCurMoveToNextWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 12);
    ctxCurMoveToNextWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 16);
    ctxCurMoveToNextWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 18);
    ctxCurMoveToNextWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 20);
    ctxCurMoveToNextWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 20);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxCurMoveToNextWordStartAlpha),
    testMake(test_ctxCurMoveToNextWordStartAlnum),
    testMake(test_ctxCurMoveToNextWordStartPunctuation),
    testMake(test_ctxCurMoveToNextWordStartMixed)
)

