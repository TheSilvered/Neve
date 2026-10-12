#include "nv_test.h"
#include "nv_context.c"

void test_ctxCurMoveFwdAscii(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcd";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    ctxCurAdd(&ctx, 2);
    ctxCurAdd(&ctx, 3);
    ctxCurMoveFwd(&ctx);

    testAssert(ctx.cursors.items[0].idx == 3);
    testAssert(ctx.cursors.items[0].anchor == 3);
    testAssert(ctx.cursors.items[1].idx == 4);
    testAssert(ctx.cursors.items[1].anchor == 4);

    ctxDestroy(&ctx);
}

void test_ctxCurMoveFwdMultiline(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncd\n";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    ctxCurAdd(&ctx, 1);
    ctxCurAdd(&ctx, 2);
    ctxCurAdd(&ctx, 4);
    ctxCurAdd(&ctx, 5);
    ctxCurMoveFwd(&ctx);

    testAssert(ctx.cursors.len == 4);
    testAssert(ctx.cursors.items[0].idx == 2);
    testAssert(ctx.cursors.items[0].anchor == 2);
    testAssert(ctx.cursors.items[1].idx == 3);
    testAssert(ctx.cursors.items[1].anchor == 0);
    testAssert(ctx.cursors.items[2].idx == 5);
    testAssert(ctx.cursors.items[2].anchor == 2);
    testAssert(ctx.cursors.items[3].idx == 6);
    testAssert(ctx.cursors.items[3].anchor == 0);

    ctxDestroy(&ctx);
}

void text_ctxCurMoveFwdUnicode(void) {
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

    ctxCurMoveFwd(&ctx);

    testAssert(ctx.cursors.len == 4);
    testAssert(ctx.cursors.items[0].idx == 2);
    testAssert(ctx.cursors.items[0].anchor == 1);
    testAssert(ctx.cursors.items[1].idx == 6);
    testAssert(ctx.cursors.items[1].anchor == 3);
    testAssert(ctx.cursors.items[2].idx == 7);
    testAssert(ctx.cursors.items[2].anchor == 8);
    testAssert(ctx.cursors.items[3].idx == 8);
    testAssert(ctx.cursors.items[3].anchor == 9);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxCurMoveFwdAscii),
    testMake(test_ctxCurMoveFwdMultiline),
    testMake(text_ctxCurMoveFwdUnicode)
)
