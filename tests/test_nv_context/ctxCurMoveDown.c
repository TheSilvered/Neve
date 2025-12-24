#include "nv_test.h"
#include "nv_context.c"

void test_ctxCurMoveDownLastLine(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcd";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    ctxCurAdd(&ctx, 1);
    ctxCurAdd(&ctx, 2);

    ctxCurMoveDown(&ctx);

    testAssert(ctx.cursors.items[0].idx == 1);
    testAssert(ctx.cursors.items[0].baseCol == 1);
    testAssert(ctx.cursors.items[1].idx == 2);
    testAssert(ctx.cursors.items[1].baseCol == 2);

    ctxDestroy(&ctx);
}

void test_ctxCurMoveDownMultiline(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcd\nefgh\njklm";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    /*
     a|b c d
     e|f g|h
    |j|k l m

    becomes

     a b c d
     e|f g h
    |j|k l|m
    */

    ctxCurAdd(&ctx, 1);
    ctxCurAdd(&ctx, 6);
    ctxCurAdd(&ctx, 8);
    ctxCurAdd(&ctx, 10);
    ctxCurAdd(&ctx, 11);

    ctxCurMoveDown(&ctx);

    testAssert(ctx.cursors.len == 4);
    testAssert(ctx.cursors.items[0].idx == 6);
    testAssert(ctx.cursors.items[0].baseCol == 1);
    testAssert(ctx.cursors.items[1].idx == 10);
    testAssert(ctx.cursors.items[1].baseCol == 0);
    testAssert(ctx.cursors.items[2].idx == 11);
    testAssert(ctx.cursors.items[2].baseCol == 1);
    testAssert(ctx.cursors.items[3].idx == 13);
    testAssert(ctx.cursors.items[3].baseCol == 3);

    ctxDestroy(&ctx);
}

void test_ctxCurMoveDownBaseCol(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcd\n\t\ne\xf0\x9f\x98\x8a\nfghi";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    ctxCurAdd(&ctx, 2);

    testAssert(ctx.cursors.items[0].baseCol == 2);
    ctxCurMoveDown(&ctx);
    testAssert(ctx.cursors.items[0].idx == 5);
    testAssert(ctx.cursors.items[0].baseCol == 2);
    ctxCurMoveDown(&ctx);
    testAssert(ctx.cursors.items[0].idx == 8);
    testAssert(ctx.cursors.items[0].baseCol == 2);
    ctxCurMoveDown(&ctx);
    testAssert(ctx.cursors.items[0].idx == 15);
    testAssert(ctx.cursors.items[0].baseCol == 2);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxCurMoveDownLastLine),
    testMake(test_ctxCurMoveDownMultiline),
    testMake(test_ctxCurMoveDownBaseCol)
)
