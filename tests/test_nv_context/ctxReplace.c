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

    ctxReplace_(&ctx, 2, 5, (UcdCh8 *)"rpl", 3);
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

    ctxReplace_(&ctx, 2, 5, (UcdCh8 *)"rp", 2);
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

    ctxReplace_(&ctx, 2, 5, NULL, 0);
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

    ctxReplace_(&ctx, 2, 5, (UcdCh8 *)"rplx", 4);
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

    ctxReplace_(&ctx, 2, 5, (UcdCh8 *)"rpl", 3);
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

    ctxReplace_(&ctx, 2, 5, (UcdCh8 *)"rp", 2);
    testAssert(ctx.cursors.len == 3);
    testAssert(ctx.cursors.items[0].idx == 2);
    testAssert(ctx.cursors.items[0].baseCol == 2);
    testAssert(ctx.cursors.items[1].idx == 4);
    testAssert(ctx.cursors.items[1].baseCol == 4);
    testAssert(ctx.cursors.items[2].idx == 5);
    testAssert(ctx.cursors.items[2].baseCol == 5);

    ctxDestroy(&ctx);
}

void test_ctxReplaceSpanEmptyWCursors(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 2);
    ctxCurAdd(&ctx, 3);
    ctxCurAdd(&ctx, 5);
    ctxCurAdd(&ctx, 6);

    ctxReplace_(&ctx, 2, 5, NULL, 0);
    testAssert(ctx.cursors.len == 2);
    testAssert(ctx.cursors.items[0].idx == 2);
    testAssert(ctx.cursors.items[0].baseCol == 2);
    testAssert(ctx.cursors.items[1].idx == 3);
    testAssert(ctx.cursors.items[1].baseCol == 3);

    ctxDestroy(&ctx);
}

void test_ctxReplaceSpanSameLenWSelections(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    // They are not normalized but ctxReplace_ does not care
    arrAppend(&ctx._sels, (CtxSelection){ 0, 2 }); // fully before
    arrAppend(&ctx._sels, (CtxSelection){ 1, 3 }); // ends inside
    arrAppend(&ctx._sels, (CtxSelection){ 1, 6 }); // surrounds
    arrAppend(&ctx._sels, (CtxSelection){ 2, 5 }); // matches
    arrAppend(&ctx._sels, (CtxSelection){ 3, 4 }); // fully inside
    arrAppend(&ctx._sels, (CtxSelection){ 4, 6 }); // starts inside
    arrAppend(&ctx._sels, (CtxSelection){ 5, 7 }); // fully after

    ctxReplace_(&ctx, 2, 5, (UcdCh8 *)"rpl", 3);
    testAssert(ctx._sels.len == 5);
    testAssert(ctx._sels.items[0].startIdx == 0);
    testAssert(ctx._sels.items[0].endIdx == 2);
    testAssert(ctx._sels.items[1].startIdx == 1);
    testAssert(ctx._sels.items[1].endIdx == 2);
    testAssert(ctx._sels.items[2].startIdx == 1);
    testAssert(ctx._sels.items[2].endIdx == 6);
    testAssert(ctx._sels.items[3].startIdx == 5);
    testAssert(ctx._sels.items[3].endIdx == 6);
    testAssert(ctx._sels.items[4].startIdx == 5);
    testAssert(ctx._sels.items[4].endIdx == 7);

    ctxDestroy(&ctx);
}

void test_ctxReplaceSpanShorterWSelections(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    // They are not normalized but ctxReplace_ does not care
    arrAppend(&ctx._sels, (CtxSelection){ 0, 2 }); // fully before
    arrAppend(&ctx._sels, (CtxSelection){ 1, 3 }); // ends inside
    arrAppend(&ctx._sels, (CtxSelection){ 1, 6 }); // surrounds
    arrAppend(&ctx._sels, (CtxSelection){ 2, 5 }); // matches
    arrAppend(&ctx._sels, (CtxSelection){ 3, 4 }); // fully inside
    arrAppend(&ctx._sels, (CtxSelection){ 4, 6 }); // starts inside
    arrAppend(&ctx._sels, (CtxSelection){ 5, 7 }); // fully after

    ctxReplace_(&ctx, 2, 5, (UcdCh8 *)"rp", 2);
    testAssert(ctx._sels.len == 5);
    testAssert(ctx._sels.items[0].startIdx == 0);
    testAssert(ctx._sels.items[0].endIdx == 2);
    testAssert(ctx._sels.items[1].startIdx == 1);
    testAssert(ctx._sels.items[1].endIdx == 2);
    testAssert(ctx._sels.items[2].startIdx == 1);
    testAssert(ctx._sels.items[2].endIdx == 5);
    testAssert(ctx._sels.items[3].startIdx == 4);
    testAssert(ctx._sels.items[3].endIdx == 5);
    testAssert(ctx._sels.items[4].startIdx == 4);
    testAssert(ctx._sels.items[4].endIdx == 6);

    ctxDestroy(&ctx);
}

void test_ctxReplaceSpanEmptyWSelections(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    // They are not normalized but ctxReplace_ does not care
    arrAppend(&ctx._sels, (CtxSelection){ 0, 2 }); // fully before
    arrAppend(&ctx._sels, (CtxSelection){ 1, 3 }); // ends inside
    arrAppend(&ctx._sels, (CtxSelection){ 1, 6 }); // surrounds
    arrAppend(&ctx._sels, (CtxSelection){ 2, 5 }); // matches
    arrAppend(&ctx._sels, (CtxSelection){ 3, 4 }); // fully inside
    arrAppend(&ctx._sels, (CtxSelection){ 4, 6 }); // starts inside
    arrAppend(&ctx._sels, (CtxSelection){ 5, 7 }); // fully after

    ctxReplace_(&ctx, 2, 5, NULL, 0);
    testAssert(ctx._sels.len == 5);
    testAssert(ctx._sels.items[0].startIdx == 0);
    testAssert(ctx._sels.items[0].endIdx == 2);
    testAssert(ctx._sels.items[1].startIdx == 1);
    testAssert(ctx._sels.items[1].endIdx == 2);
    testAssert(ctx._sels.items[2].startIdx == 1);
    testAssert(ctx._sels.items[2].endIdx == 3);
    testAssert(ctx._sels.items[3].startIdx == 2);
    testAssert(ctx._sels.items[3].endIdx == 3);
    testAssert(ctx._sels.items[4].startIdx == 2);
    testAssert(ctx._sels.items[4].endIdx == 4);

    ctxDestroy(&ctx);
}

void test_ctxReplaceSpanEmptyJoinSelections(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    arrAppend(&ctx._sels, (CtxSelection){ 1, 3 });
    arrAppend(&ctx._sels, (CtxSelection){ 4, 6 });

    ctxReplace_(&ctx, 2, 5, NULL, 0);
    testAssert(ctx._sels.len == 1);
    testAssert(ctx._sels.items[0].startIdx == 1);
    testAssert(ctx._sels.items[0].endIdx == 3);

    ctxDestroy(&ctx);
}

void test_ctxReplaceSpanLongerWSelections(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    // They are not normalized but ctxReplace_ does not care
    arrAppend(&ctx._sels, (CtxSelection){ 0, 2 }); // fully before
    arrAppend(&ctx._sels, (CtxSelection){ 1, 3 }); // ends inside
    arrAppend(&ctx._sels, (CtxSelection){ 1, 6 }); // surrounds
    arrAppend(&ctx._sels, (CtxSelection){ 2, 5 }); // matches
    arrAppend(&ctx._sels, (CtxSelection){ 3, 4 }); // fully inside
    arrAppend(&ctx._sels, (CtxSelection){ 4, 6 }); // starts inside
    arrAppend(&ctx._sels, (CtxSelection){ 5, 7 }); // fully after

    ctxReplace_(&ctx, 2, 5, (UcdCh8 *)"rplx", 4);
    testAssert(ctx._sels.len == 5);
    testAssert(ctx._sels.items[0].startIdx == 0);
    testAssert(ctx._sels.items[0].endIdx == 2);
    testAssert(ctx._sels.items[1].startIdx == 1);
    testAssert(ctx._sels.items[1].endIdx == 2);
    testAssert(ctx._sels.items[2].startIdx == 1);
    testAssert(ctx._sels.items[2].endIdx == 7);
    testAssert(ctx._sels.items[3].startIdx == 6);
    testAssert(ctx._sels.items[3].endIdx == 7);
    testAssert(ctx._sels.items[4].startIdx == 6);
    testAssert(ctx._sels.items[4].endIdx == 8);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxReplaceSpanSameLen),
    testMake(test_ctxReplaceSpanShorter),
    testMake(test_ctxReplaceSpanEmpty),
    testMake(test_ctxReplaceSpanLonger),
    testMake(test_ctxReplaceSpanSameLenWCursors),
    testMake(test_ctxReplaceSpanShorterWCursors),
    testMake(test_ctxReplaceSpanEmptyWCursors),
    testMake(test_ctxReplaceSpanSameLenWSelections),
    testMake(test_ctxReplaceSpanShorterWSelections),
    testMake(test_ctxReplaceSpanEmptyWSelections),
    testMake(test_ctxReplaceSpanEmptyJoinSelections),
    testMake(test_ctxReplaceSpanLongerWSelections)
)
