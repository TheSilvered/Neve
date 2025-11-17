#include "nv_context.h"
#include "nv_test.h"
#include "nv_context.c"

void test_ctxCurMoveToPrevWordStartAlpha(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abc \tdef\nghijkl";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 15);
    ctxCurMoveToPrevWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 9);
    ctxCurMoveToPrevWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 5);
    ctxCurMoveToPrevWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 0);
    ctxCurMoveToPrevWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 0);

    ctxDestroy(&ctx);
}

void test_ctxCurMoveToPrevWordStartAlnum(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "a9c \t123\n2hijk5";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 15);
    ctxCurMoveToPrevWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 9);
    ctxCurMoveToPrevWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 5);
    ctxCurMoveToPrevWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 0);
    ctxCurMoveToPrevWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 0);

    ctxDestroy(&ctx);
}

void test_ctxCurMoveToPrevWordStartPunctuation(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "()* \t.,-\n&%/!:;";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 15);
    ctxCurMoveToPrevWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 9);
    ctxCurMoveToPrevWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 5);
    ctxCurMoveToPrevWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 0);
    ctxCurMoveToPrevWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 0);

    ctxDestroy(&ctx);
}

void test_ctxCurMoveToPrevWordStartMixed(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abc9()*  ==?92\n\t& ab";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 20);
    ctxCurMoveToPrevWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 18);
    ctxCurMoveToPrevWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 16);
    ctxCurMoveToPrevWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 12);
    ctxCurMoveToPrevWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 9);
    ctxCurMoveToPrevWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 4);
    ctxCurMoveToPrevWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 0);
    ctxCurMoveToPrevWordStart(&ctx);
    testAssert(ctx.cursors.items[0].idx == 0);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxCurMoveToPrevWordStartAlpha),
    testMake(test_ctxCurMoveToPrevWordStartAlnum),
    testMake(test_ctxCurMoveToPrevWordStartPunctuation),
    testMake(test_ctxCurMoveToPrevWordStartMixed)
)
