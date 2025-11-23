#include "nv_test.h"
#define lineRefMaxGap_ 8
#include "nv_context.c"

bool eqStrViewCStr(StrView *sv, const char *cStr) {
    return strncmp((char *)sv->buf, cStr, sv->len) == 0;
}

void test_ctxReplaceSpanSameLen(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxReplaceSpan_(&ctx, 2, 5, (UcdCh8 *)"rpl", 3);
    StrView content = ctxGetContent(&ctx);
    testAssert(eqStrViewCStr(&content, "abrplfg"));
    testAssert(content.len == 7);
    testAssert(ctx._refs.len == 0);

    ctxDestroy(&ctx);
}

void test_ctxReplaceSpanShorter(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxReplaceSpan_(&ctx, 2, 5, (UcdCh8 *)"rp", 2);
    StrView content = ctxGetContent(&ctx);
    testAssert(eqStrViewCStr(&content, "abrpfg"));
    testAssert(content.len == 6);
    testAssert(ctx._refs.len == 0);

    ctxDestroy(&ctx);
}

void test_ctxReplaceSpanEmpty(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxReplaceSpan_(&ctx, 2, 5, NULL, 0);
    StrView content = ctxGetContent(&ctx);
    testAssert(eqStrViewCStr(&content, "abfg"));
    testAssert(content.len == 4);
    testAssert(ctx._refs.len == 0);

    ctxDestroy(&ctx);
}

void test_ctxReplaceSpanLonger(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxReplaceSpan_(&ctx, 2, 5, (UcdCh8 *)"rplx", 4);
    StrView content = ctxGetContent(&ctx);
    testAssert(eqStrViewCStr(&content, "abrplxfg"));
    testAssert(content.len == 8);
    testAssert(ctx._refs.len == 0);

    ctxDestroy(&ctx);
}

void test_ctxReplaceSpanSameLenWCursors(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 2);
    ctxCurAdd(&ctx, 3);
    ctxCurAdd(&ctx, 5);
    ctxCurAdd(&ctx, 6);

    ctxReplaceSpan_(&ctx, 2, 5, (UcdCh8 *)"rpl", 3);
    testAssert(ctx.cursors.len == 3);
    testAssert(ctx.cursors.items[0].idx == 2);
    testAssert(ctx.cursors.items[0].baseCol == 2);
    testAssert(ctx.cursors.items[1].idx == 5);
    testAssert(ctx.cursors.items[1].baseCol == 5);
    testAssert(ctx.cursors.items[2].idx == 6);
    testAssert(ctx.cursors.items[2].baseCol == 6);

    ctxDestroy(&ctx);
}

void test_ctxReplaceSpanShorterWCursors(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 2);
    ctxCurAdd(&ctx, 3);
    ctxCurAdd(&ctx, 5);
    ctxCurAdd(&ctx, 6);

    ctxReplaceSpan_(&ctx, 2, 5, (UcdCh8 *)"rp", 2);
    testAssert(ctx.cursors.len == 3);
    testAssert(ctx.cursors.items[0].idx == 2);
    testAssert(ctx.cursors.items[0].baseCol == 2);
    testAssert(ctx.cursors.items[1].idx == 4);
    testAssert(ctx.cursors.items[1].baseCol == 4);
    testAssert(ctx.cursors.items[2].idx == 5);
    testAssert(ctx.cursors.items[2].baseCol == 5);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxReplaceSpanSameLen),
    testMake(test_ctxReplaceSpanShorter),
    testMake(test_ctxReplaceSpanEmpty),
    testMake(test_ctxReplaceSpanLonger),
    testMake(test_ctxReplaceSpanSameLenWCursors),
    testMake(test_ctxReplaceSpanShorterWCursors),
)
