#include "nv_test.h"
#include "nv_context.c"

void test_ctxCurMoveBackAscii(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcd";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 1);
    ctxCurAdd(&ctx, 2);
    ctxCurMoveBack(&ctx);

    testAssert(ctx.cursors.items[0].idx == 0);
    testAssert(ctx.cursors.items[0].baseCol == 0);
    testAssert(ctx.cursors.items[1].idx == 1);
    testAssert(ctx.cursors.items[1].baseCol == 1);

    ctxDestroy(&ctx);
}

void test_ctxCurMoveBackMultiline(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncd\n";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 0);
    ctxCurAdd(&ctx, 1);
    ctxCurAdd(&ctx, 3);
    ctxCurAdd(&ctx, 4);
    ctxCurMoveBack(&ctx);

    testAssert(ctx.cursors.len == 3);
    testAssert(ctx.cursors.items[0].idx == 0);
    testAssert(ctx.cursors.items[0].baseCol == 0);
    testAssert(ctx.cursors.items[1].idx == 2);
    testAssert(ctx.cursors.items[1].baseCol == 2);
    testAssert(ctx.cursors.items[2].idx == 3);
    testAssert(ctx.cursors.items[2].baseCol == 0);

    ctxDestroy(&ctx);
}

void text_ctxCurMoveBackUnicode(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "\xc3\xa8\xf0\x9f\x98\x8a\ta";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 0);
    ctxCurAdd(&ctx, 2);
    ctxCurAdd(&ctx, 6);
    ctxCurAdd(&ctx, 7);
    ctxCurAdd(&ctx, 8);

    testAssert(ctx.cursors.items[0].baseCol == 0);
    testAssert(ctx.cursors.items[1].baseCol == 1);
    testAssert(ctx.cursors.items[2].baseCol == 3);
    testAssert(ctx.cursors.items[3].baseCol == 8);
    testAssert(ctx.cursors.items[4].baseCol == 9);

    ctxCurMoveBack(&ctx);

    testAssert(ctx.cursors.len == 4);
    testAssert(ctx.cursors.items[0].idx == 0);
    testAssert(ctx.cursors.items[0].baseCol == 0);
    testAssert(ctx.cursors.items[1].idx == 2);
    testAssert(ctx.cursors.items[1].baseCol == 1);
    testAssert(ctx.cursors.items[2].idx == 6);
    testAssert(ctx.cursors.items[2].baseCol == 3);
    testAssert(ctx.cursors.items[3].idx == 7);
    testAssert(ctx.cursors.items[3].baseCol == 8);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxCurMoveBackAscii),
    testMake(test_ctxCurMoveBackMultiline),
    testMake(text_ctxCurMoveBackUnicode)
)
