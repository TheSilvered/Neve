#include "nv_test.h"
#define lineRefMaxGap_ 8
#include "nv_context.c"

bool eqStrViewCStr(StrView *sv, const char *cStr) {
    return strncmp((char *)sv->buf, cStr, sv->len) == 0;
}

void test_ctxReplaceSameLen(void) {
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

void test_ctxReplaceShorter(void) {
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

void test_ctxReplaceEmpty(void) {
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

void test_ctxReplaceLonger(void) {
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

void test_ctxReplaceSameLenWCursors(void) {
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

void test_ctxReplaceShorterWCursors(void) {
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

void test_ctxReplaceEmptyWCursors(void) {
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

void test_ctxReplaceLongerWCursors(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 2);
    ctxCurAdd(&ctx, 3);
    ctxCurAdd(&ctx, 5);
    ctxCurAdd(&ctx, 6);

    ctxReplace_(&ctx, 2, 5, (UcdCh8 *)"rplx", 4);
    testAssert(ctx.cursors.len == 3);
    testAssert(ctx.cursors.items[0].idx == 2);
    testAssert(ctx.cursors.items[0].baseCol == 2);
    testAssert(ctx.cursors.items[1].idx == 6);
    testAssert(ctx.cursors.items[1].baseCol == 6);
    testAssert(ctx.cursors.items[2].idx == 7);
    testAssert(ctx.cursors.items[2].baseCol == 7);

    ctxDestroy(&ctx);
}

void test_ctxReplaceSameLenWSelCursors(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 1);
    ctxCurAdd(&ctx, 7);
    ctxCurAdd(&ctx, 3);
    ctxCurAdd(&ctx, 4);

    ctxSelBegin(&ctx);

    ctxCurMove(&ctx, 1, 6);
    ctxCurMove(&ctx, 7, 1);
    ctxCurMove(&ctx, 3, 0);
    ctxCurMove(&ctx, 4, 7);

    ctxReplace_(&ctx, 2, 5, (UcdCh8 *)"rpl", 3);
    testAssert(ctx.cursors.len == 4);
    testAssert(ctx.cursors.items[0].idx == 0);
    testAssert(ctx.cursors.items[0]._selStart == 2);
    testAssert(ctx.cursors.items[1].idx == 1);
    testAssert(ctx.cursors.items[1]._selStart == 7);
    testAssert(ctx.cursors.items[2].idx == 6);
    testAssert(ctx.cursors.items[2]._selStart == 1);
    testAssert(ctx.cursors.items[3].idx == 7);
    testAssert(ctx.cursors.items[3]._selStart == 5);

    ctxDestroy(&ctx);
}

void test_ctxReplaceShorterWSelCursors(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 1);
    ctxCurAdd(&ctx, 7);
    ctxCurAdd(&ctx, 3);
    ctxCurAdd(&ctx, 4);

    ctxSelBegin(&ctx);

    ctxCurMove(&ctx, 1, 6);
    ctxCurMove(&ctx, 7, 1);
    ctxCurMove(&ctx, 3, 0);
    ctxCurMove(&ctx, 4, 7);

    ctxReplace_(&ctx, 2, 5, (UcdCh8 *)"rp", 2);
    testAssert(ctx.cursors.len == 4);
    testAssert(ctx.cursors.items[0].idx == 0);
    testAssert(ctx.cursors.items[0]._selStart == 2);
    testAssert(ctx.cursors.items[1].idx == 1);
    testAssert(ctx.cursors.items[1]._selStart == 6);
    testAssert(ctx.cursors.items[2].idx == 5);
    testAssert(ctx.cursors.items[2]._selStart == 1);
    testAssert(ctx.cursors.items[3].idx == 6);
    testAssert(ctx.cursors.items[3]._selStart == 4);

    ctxDestroy(&ctx);
}

void test_ctxReplaceEmptyWSelCursors(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 1);
    ctxCurAdd(&ctx, 7);
    ctxCurAdd(&ctx, 3);
    ctxCurAdd(&ctx, 4);

    ctxSelBegin(&ctx);

    ctxCurMove(&ctx, 1, 6);
    ctxCurMove(&ctx, 7, 1);
    ctxCurMove(&ctx, 3, 0);
    ctxCurMove(&ctx, 4, 7);

    ctxReplace_(&ctx, 2, 5, NULL, 0);
    testAssert(ctx.cursors.len == 4);
    testAssert(ctx.cursors.items[0].idx == 0);
    testAssert(ctx.cursors.items[0]._selStart == 2);
    testAssert(ctx.cursors.items[1].idx == 1);
    testAssert(ctx.cursors.items[1]._selStart == 4);
    testAssert(ctx.cursors.items[2].idx == 3);
    testAssert(ctx.cursors.items[2]._selStart == 1);
    testAssert(ctx.cursors.items[3].idx == 4);
    testAssert(ctx.cursors.items[3]._selStart == 2);

    ctxDestroy(&ctx);
}

void test_ctxReplaceLongerWSelCursors(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 1);
    ctxCurAdd(&ctx, 7);
    ctxCurAdd(&ctx, 3);
    ctxCurAdd(&ctx, 4);

    ctxSelBegin(&ctx);

    ctxCurMove(&ctx, 1, 6);
    ctxCurMove(&ctx, 7, 1);
    ctxCurMove(&ctx, 3, 0);
    ctxCurMove(&ctx, 4, 7);

    ctxReplace_(&ctx, 2, 5, (UcdCh8 *)"rplx", 4);
    testAssert(ctx.cursors.len == 4);
    testAssert(ctx.cursors.items[0].idx == 0);
    testAssert(ctx.cursors.items[0]._selStart == 2);
    testAssert(ctx.cursors.items[1].idx == 1);
    testAssert(ctx.cursors.items[1]._selStart == 8);
    testAssert(ctx.cursors.items[2].idx == 7);
    testAssert(ctx.cursors.items[2]._selStart == 1);
    testAssert(ctx.cursors.items[3].idx == 8);
    testAssert(ctx.cursors.items[3]._selStart == 6);

    ctxDestroy(&ctx);
}

void test_ctxReplaceSameLenWSelections(void) {
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

void test_ctxReplaceShorterWSelections(void) {
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

void test_ctxReplaceEmptyWSelections(void) {
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

void test_ctxReplaceEmptyJoinSelections(void) {
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

void test_ctxReplaceLongerWSelections(void) {
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
    testMake(test_ctxReplaceSameLen),
    testMake(test_ctxReplaceShorter),
    testMake(test_ctxReplaceEmpty),
    testMake(test_ctxReplaceLonger),

    testMake(test_ctxReplaceSameLenWCursors),
    testMake(test_ctxReplaceShorterWCursors),
    testMake(test_ctxReplaceEmptyWCursors),
    testMake(test_ctxReplaceLongerWCursors),

    testMake(test_ctxReplaceSameLenWSelCursors),
    testMake(test_ctxReplaceShorterWSelCursors),
    testMake(test_ctxReplaceEmptyWSelCursors),
    testMake(test_ctxReplaceLongerWSelCursors),

    testMake(test_ctxReplaceSameLenWSelections),
    testMake(test_ctxReplaceShorterWSelections),
    testMake(test_ctxReplaceEmptyWSelections),
    testMake(test_ctxReplaceEmptyJoinSelections),
    testMake(test_ctxReplaceLongerWSelections)
)
