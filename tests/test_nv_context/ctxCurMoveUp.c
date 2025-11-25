#include "nv_test.h"
#include "nv_context.c"

void test_ctxCurMoveUpFirstLine(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcd";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 1);
    ctxCurAdd(&ctx, 2);

    ctxCurMoveUp(&ctx);

    testAssert(ctx.cursors.items[0].idx == 1);
    testAssert(ctx.cursors.items[0].baseCol == 1);
    testAssert(ctx.cursors.items[1].idx == 2);
    testAssert(ctx.cursors.items[1].baseCol == 2);

    ctxDestroy(&ctx);
}

void test_ctxCurMoveUpMultiline(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcd\nefgh\njklm";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    /*
     a|b c d
     e|f g|h
    |j|k l m

    becomes

     a|b c|d
    |e|f g h
     j k l m
    */

    ctxCurAdd(&ctx, 1);
    ctxCurAdd(&ctx, 6);
    ctxCurAdd(&ctx, 8);
    ctxCurAdd(&ctx, 10);
    ctxCurAdd(&ctx, 11);

    ctxCurMoveUp(&ctx);

    testAssert(ctx.cursors.len == 4);
    testAssert(ctx.cursors.items[0].idx == 1);
    testAssert(ctx.cursors.items[0].baseCol == 1);
    testAssert(ctx.cursors.items[1].idx == 3);
    testAssert(ctx.cursors.items[1].baseCol == 3);
    testAssert(ctx.cursors.items[2].idx == 5);
    testAssert(ctx.cursors.items[2].baseCol == 0);
    testAssert(ctx.cursors.items[3].idx == 6);
    testAssert(ctx.cursors.items[3].baseCol == 1);

    ctxDestroy(&ctx);
}

void test_ctxCurMoveUpBaseCol(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcd\n\t\ne\xf0\x9f\x98\x8a\nfghi";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 15);

    testAssert(ctx.cursors.items[0].baseCol == 2);
    ctxCurMoveUp(&ctx);
    testAssert(ctx.cursors.items[0].idx == 8);
    testAssert(ctx.cursors.items[0].baseCol == 2);
    ctxCurMoveUp(&ctx);
    testAssert(ctx.cursors.items[0].idx == 5);
    testAssert(ctx.cursors.items[0].baseCol == 2);
    ctxCurMoveUp(&ctx);
    testAssert(ctx.cursors.items[0].idx == 2);
    testAssert(ctx.cursors.items[0].baseCol == 2);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxCurMoveUpFirstLine),
    testMake(test_ctxCurMoveUpMultiline),
    testMake(test_ctxCurMoveUpBaseCol)
)
