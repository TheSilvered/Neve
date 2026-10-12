#include "nv_test.h"
#include "nv_context.c"

void test_ctxCurMoveLeftAscii(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcd";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    ctxCurAdd(&ctx, 1);
    ctxCurAdd(&ctx, 2);
    ctxCurMoveLeft(&ctx);

    testAssert(ctx.cursors.items[0].idx == 0);
    testAssert(ctx.cursors.items[0].anchor == 0);
    testAssert(ctx.cursors.items[1].idx == 1);
    testAssert(ctx.cursors.items[1].anchor == 1);

    ctxDestroy(&ctx);
}

void test_ctxCurMoveLeftMultiline(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncd\n";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    ctxCurAdd(&ctx, 0);
    ctxCurAdd(&ctx, 1);
    ctxCurAdd(&ctx, 3);
    ctxCurAdd(&ctx, 4);
    ctxCurMoveLeft(&ctx);

    testAssert(ctx.cursors.len == 2);
    testAssert(ctx.cursors.items[0].idx == 0);
    testAssert(ctx.cursors.items[0].anchor == 0);
    testAssert(ctx.cursors.items[1].idx == 3);
    testAssert(ctx.cursors.items[1].anchor == 0);

    ctxDestroy(&ctx);
}

void text_ctxCurMoveLeftUnicode(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "\xc3\xa8\xf0\x9f\x98\x8a\ta";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    ctxCurAdd(&ctx, 0);
    ctxCurAdd(&ctx, 2);
    ctxCurAdd(&ctx, 6);
    ctxCurAdd(&ctx, 7);
    ctxCurAdd(&ctx, 8);

    testAssert(ctx.cursors.items[0].anchor == 0);
    testAssert(ctx.cursors.items[1].anchor == 1);
    testAssert(ctx.cursors.items[2].anchor == 3);
    testAssert(ctx.cursors.items[3].anchor == 8);
    testAssert(ctx.cursors.items[4].anchor == 9);

    ctxCurMoveLeft(&ctx);

    testAssert(ctx.cursors.len == 4);
    testAssert(ctx.cursors.items[0].idx == 0);
    testAssert(ctx.cursors.items[0].anchor == 0);
    testAssert(ctx.cursors.items[1].idx == 2);
    testAssert(ctx.cursors.items[1].anchor == 1);
    testAssert(ctx.cursors.items[2].idx == 6);
    testAssert(ctx.cursors.items[2].anchor == 3);
    testAssert(ctx.cursors.items[3].idx == 7);
    testAssert(ctx.cursors.items[3].anchor == 8);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxCurMoveLeftAscii),
    testMake(test_ctxCurMoveLeftMultiline),
    testMake(text_ctxCurMoveLeftUnicode)
)
