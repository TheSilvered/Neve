#include "nv_test.h"
#define _lineRefMaxGap 8
#include "nv_context.c"

bool eqStrViewCStr(StrView sv, const char *cStr) {
    return strncmp((char *)sv.buf, cStr, sv.len) == 0;
}

void test_ctxReplaceSameLen(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    _ctxReplace(&ctx, 2, 5, (Utf8Ch *)"rpl", 3);
    StrView content = ctxGetContent(&ctx);
    testAssert(eqStrViewCStr(content, "abrplfg"));
    testAssert(content.len == 7);
    testAssert(ctx._refs.len == 0);

    ctxDestroy(&ctx);
}

void test_ctxReplaceShorter(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    _ctxReplace(&ctx, 2, 5, (Utf8Ch *)"rp", 2);
    StrView content = ctxGetContent(&ctx);
    testAssert(eqStrViewCStr(content, "abrpfg"));
    testAssert(content.len == 6);
    testAssert(ctx._refs.len == 0);

    ctxDestroy(&ctx);
}

void test_ctxReplaceEmpty(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    _ctxReplace(&ctx, 2, 5, NULL, 0);
    StrView content = ctxGetContent(&ctx);
    testAssert(eqStrViewCStr(content, "abfg"));
    testAssert(content.len == 4);
    testAssert(ctx._refs.len == 0);

    ctxDestroy(&ctx);
}

void test_ctxReplaceLonger(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    _ctxReplace(&ctx, 2, 5, (Utf8Ch *)"rplx", 4);
    StrView content = ctxGetContent(&ctx);
    testAssert(eqStrViewCStr(content, "abrplxfg"));
    testAssert(content.len == 8);
    testAssert(ctx._refs.len == 0);

    ctxDestroy(&ctx);
}

void test_ctxReplaceAtBeginning(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    _ctxReplace(&ctx, 0, 3, (Utf8Ch *)"rplx", 4);

    StrView content = ctxGetContent(&ctx);
    testAssert(eqStrViewCStr(content, "rplxdefg"));
    testAssert(content.len == 8);
    testAssert(ctx._refs.len == 0);

    ctxDestroy(&ctx);
}

void test_ctxReplaceAtEnd(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    _ctxReplace(&ctx, 4, 7, (Utf8Ch *)"rplx", 4);

    StrView content = ctxGetContent(&ctx);
    testAssert(eqStrViewCStr(content, "abcdrplx"));
    testAssert(content.len == 8);
    testAssert(ctx._refs.len == 1);
    testAssert(ctx._refs.items[0].idx == 8);
    testAssert(ctx._refs.items[0].col == 8);
    testAssert(ctx._refs.items[0].line == 0);

    ctxDestroy(&ctx);
}

void test_ctxReplaceSameLenWCursors(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    ctxCurAdd(&ctx, 2);
    ctxCurAdd(&ctx, 3);
    ctxCurAdd(&ctx, 5);
    ctxCurAdd(&ctx, 6);

    _ctxReplace(&ctx, 2, 5, (Utf8Ch *)"rpl", 3);
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
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    ctxCurAdd(&ctx, 2);
    ctxCurAdd(&ctx, 3);
    ctxCurAdd(&ctx, 5);
    ctxCurAdd(&ctx, 6);

    _ctxReplace(&ctx, 2, 5, (Utf8Ch *)"rp", 2);
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
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    ctxCurAdd(&ctx, 2);
    ctxCurAdd(&ctx, 3);
    ctxCurAdd(&ctx, 5);
    ctxCurAdd(&ctx, 6);

    _ctxReplace(&ctx, 2, 5, NULL, 0);
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
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    ctxCurAdd(&ctx, 2);
    ctxCurAdd(&ctx, 3);
    ctxCurAdd(&ctx, 5);
    ctxCurAdd(&ctx, 6);

    _ctxReplace(&ctx, 2, 5, (Utf8Ch *)"rplx", 4);
    testAssert(ctx.cursors.len == 3);
    testAssert(ctx.cursors.items[0].idx == 2);
    testAssert(ctx.cursors.items[0].baseCol == 2);
    testAssert(ctx.cursors.items[1].idx == 6);
    testAssert(ctx.cursors.items[1].baseCol == 6);
    testAssert(ctx.cursors.items[2].idx == 7);
    testAssert(ctx.cursors.items[2].baseCol == 7);

    ctxDestroy(&ctx);
}

void test_ctxReplaceWSingleCursor(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    ctxCurAdd(&ctx, 3);

    _ctxReplace(&ctx, 2, 5, (Utf8Ch *)"rpl", 3);
    testAssert(ctx.cursors.len == 1);
    testAssert(ctx.cursors.items[0].idx == 5);
    testAssert(ctx.cursors.items[0].baseCol = 5);

    ctxDestroy(&ctx);
}

void test_ctxReplaceSameLenWSelCursors(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    ctxCurAdd(&ctx, 1);
    ctxCurAdd(&ctx, 7);
    ctxCurAdd(&ctx, 3);
    ctxCurAdd(&ctx, 4);

    ctxSelBegin(&ctx, false);

    ctxCurMove(&ctx, 1, 6);
    ctxCurMove(&ctx, 7, 1);
    ctxCurMove(&ctx, 3, 0);
    ctxCurMove(&ctx, 4, 7);

    _ctxReplace(&ctx, 2, 5, (Utf8Ch *)"rpl", 3);
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
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    ctxCurAdd(&ctx, 1);
    ctxCurAdd(&ctx, 7);
    ctxCurAdd(&ctx, 3);
    ctxCurAdd(&ctx, 4);

    ctxSelBegin(&ctx, false);

    ctxCurMove(&ctx, 1, 6);
    ctxCurMove(&ctx, 7, 1);
    ctxCurMove(&ctx, 3, 0);
    ctxCurMove(&ctx, 4, 7);

    _ctxReplace(&ctx, 2, 5, (Utf8Ch *)"rp", 2);
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
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    ctxCurAdd(&ctx, 1);
    ctxCurAdd(&ctx, 7);
    ctxCurAdd(&ctx, 3);
    ctxCurAdd(&ctx, 4);

    ctxSelBegin(&ctx, false);

    ctxCurMove(&ctx, 1, 6);
    ctxCurMove(&ctx, 7, 1);
    ctxCurMove(&ctx, 3, 0);
    ctxCurMove(&ctx, 4, 7);

    _ctxReplace(&ctx, 2, 5, NULL, 0);
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
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    ctxCurAdd(&ctx, 1);
    ctxCurAdd(&ctx, 7);
    ctxCurAdd(&ctx, 3);
    ctxCurAdd(&ctx, 4);

    ctxSelBegin(&ctx, false);

    ctxCurMove(&ctx, 1, 6);
    ctxCurMove(&ctx, 7, 1);
    ctxCurMove(&ctx, 3, 0);
    ctxCurMove(&ctx, 4, 7);

    _ctxReplace(&ctx, 2, 5, (Utf8Ch *)"rplx", 4);
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
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    // They are not normalized but _ctxReplace does not care
    arrAppend(&ctx.sels, (CtxSelection){ 0, 2 }); // fully before
    arrAppend(&ctx.sels, (CtxSelection){ 1, 3 }); // ends inside
    arrAppend(&ctx.sels, (CtxSelection){ 1, 6 }); // surrounds
    arrAppend(&ctx.sels, (CtxSelection){ 2, 5 }); // matches
    arrAppend(&ctx.sels, (CtxSelection){ 3, 4 }); // fully inside
    arrAppend(&ctx.sels, (CtxSelection){ 4, 6 }); // starts inside
    arrAppend(&ctx.sels, (CtxSelection){ 5, 7 }); // fully after

    _ctxReplace(&ctx, 2, 5, (Utf8Ch *)"rpl", 3);
    testAssert(ctx.sels.len == 5);
    testAssert(ctx.sels.items[0].startIdx == 0);
    testAssert(ctx.sels.items[0].endIdx == 2);
    testAssert(ctx.sels.items[1].startIdx == 1);
    testAssert(ctx.sels.items[1].endIdx == 2);
    testAssert(ctx.sels.items[2].startIdx == 1);
    testAssert(ctx.sels.items[2].endIdx == 6);
    testAssert(ctx.sels.items[3].startIdx == 5);
    testAssert(ctx.sels.items[3].endIdx == 6);
    testAssert(ctx.sels.items[4].startIdx == 5);
    testAssert(ctx.sels.items[4].endIdx == 7);

    ctxDestroy(&ctx);
}

void test_ctxReplaceShorterWSelections(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    // They are not normalized but _ctxReplace does not care
    arrAppend(&ctx.sels, (CtxSelection){ 0, 2 }); // fully before
    arrAppend(&ctx.sels, (CtxSelection){ 1, 3 }); // ends inside
    arrAppend(&ctx.sels, (CtxSelection){ 1, 6 }); // surrounds
    arrAppend(&ctx.sels, (CtxSelection){ 2, 5 }); // matches
    arrAppend(&ctx.sels, (CtxSelection){ 3, 4 }); // fully inside
    arrAppend(&ctx.sels, (CtxSelection){ 4, 6 }); // starts inside
    arrAppend(&ctx.sels, (CtxSelection){ 5, 7 }); // fully after

    _ctxReplace(&ctx, 2, 5, (Utf8Ch *)"rp", 2);
    testAssert(ctx.sels.len == 5);
    testAssert(ctx.sels.items[0].startIdx == 0);
    testAssert(ctx.sels.items[0].endIdx == 2);
    testAssert(ctx.sels.items[1].startIdx == 1);
    testAssert(ctx.sels.items[1].endIdx == 2);
    testAssert(ctx.sels.items[2].startIdx == 1);
    testAssert(ctx.sels.items[2].endIdx == 5);
    testAssert(ctx.sels.items[3].startIdx == 4);
    testAssert(ctx.sels.items[3].endIdx == 5);
    testAssert(ctx.sels.items[4].startIdx == 4);
    testAssert(ctx.sels.items[4].endIdx == 6);

    ctxDestroy(&ctx);
}

void test_ctxReplaceEmptyWSelections(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    // They are not normalized but _ctxReplace does not care
    arrAppend(&ctx.sels, (CtxSelection){ 0, 2 }); // fully before
    arrAppend(&ctx.sels, (CtxSelection){ 1, 3 }); // ends inside
    arrAppend(&ctx.sels, (CtxSelection){ 1, 6 }); // surrounds
    arrAppend(&ctx.sels, (CtxSelection){ 2, 5 }); // matches
    arrAppend(&ctx.sels, (CtxSelection){ 3, 4 }); // fully inside
    arrAppend(&ctx.sels, (CtxSelection){ 4, 6 }); // starts inside
    arrAppend(&ctx.sels, (CtxSelection){ 5, 7 }); // fully after

    _ctxReplace(&ctx, 2, 5, NULL, 0);
    testAssert(ctx.sels.len == 5);
    testAssert(ctx.sels.items[0].startIdx == 0);
    testAssert(ctx.sels.items[0].endIdx == 2);
    testAssert(ctx.sels.items[1].startIdx == 1);
    testAssert(ctx.sels.items[1].endIdx == 2);
    testAssert(ctx.sels.items[2].startIdx == 1);
    testAssert(ctx.sels.items[2].endIdx == 3);
    testAssert(ctx.sels.items[3].startIdx == 2);
    testAssert(ctx.sels.items[3].endIdx == 3);
    testAssert(ctx.sels.items[4].startIdx == 2);
    testAssert(ctx.sels.items[4].endIdx == 4);

    ctxDestroy(&ctx);
}

void test_ctxReplaceEmptyJoinSelections(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    arrAppend(&ctx.sels, (CtxSelection){ 1, 3 });
    arrAppend(&ctx.sels, (CtxSelection){ 4, 6 });

    _ctxReplace(&ctx, 2, 5, NULL, 0);
    testAssert(ctx.sels.len == 1);
    testAssert(ctx.sels.items[0].startIdx == 1);
    testAssert(ctx.sels.items[0].endIdx == 3);

    ctxDestroy(&ctx);
}

void test_ctxReplaceLongerWSelections(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcdefg";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    // They are not normalized but _ctxReplace does not care
    arrAppend(&ctx.sels, (CtxSelection){ 0, 2 }); // fully before
    arrAppend(&ctx.sels, (CtxSelection){ 1, 3 }); // ends inside
    arrAppend(&ctx.sels, (CtxSelection){ 1, 6 }); // surrounds
    arrAppend(&ctx.sels, (CtxSelection){ 2, 5 }); // matches
    arrAppend(&ctx.sels, (CtxSelection){ 3, 4 }); // fully inside
    arrAppend(&ctx.sels, (CtxSelection){ 4, 6 }); // starts inside
    arrAppend(&ctx.sels, (CtxSelection){ 5, 7 }); // fully after

    _ctxReplace(&ctx, 2, 5, (Utf8Ch *)"rplx", 4);
    testAssert(ctx.sels.len == 5);
    testAssert(ctx.sels.items[0].startIdx == 0);
    testAssert(ctx.sels.items[0].endIdx == 2);
    testAssert(ctx.sels.items[1].startIdx == 1);
    testAssert(ctx.sels.items[1].endIdx == 2);
    testAssert(ctx.sels.items[2].startIdx == 1);
    testAssert(ctx.sels.items[2].endIdx == 7);
    testAssert(ctx.sels.items[3].startIdx == 6);
    testAssert(ctx.sels.items[3].endIdx == 7);
    testAssert(ctx.sels.items[4].startIdx == 6);
    testAssert(ctx.sels.items[4].endIdx == 8);

    ctxDestroy(&ctx);
}

void test_ctxReplaceSameLenCacheUpdate(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "01234567abc";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    testAssert(ctx._refs.len == 1);
    testAssert(ctx._refs.items[0].idx == 8);
    testAssert(ctx._refs.items[0].line == 0);
    testAssert(ctx._refs.items[0].col == 8);

    _ctxReplace(&ctx, 2, 5, (Utf8Ch *)"rpl", 3);

    testAssert(ctx._refs.len == 1);
    testAssert(ctx._refs.items[0].idx == 8);
    testAssert(ctx._refs.items[0].line == 0);
    testAssert(ctx._refs.items[0].col == 8);

    ctxDestroy(&ctx);
}

void test_ctxReplaceShorterCacheUpdate(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "01234567abc";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    testAssert(ctx._refs.len == 1);
    testAssert(ctx._refs.items[0].idx == 8);
    testAssert(ctx._refs.items[0].line == 0);
    testAssert(ctx._refs.items[0].col == 8);

    _ctxReplace(&ctx, 2, 5, (Utf8Ch *)"rp", 2);

    testAssert(ctx._refs.len == 1);
    testAssert(ctx._refs.items[0].idx == 7);
    testAssert(ctx._refs.items[0].line == 0);
    testAssert(ctx._refs.items[0].col == 7);

    ctxDestroy(&ctx);
}

void test_ctxReplaceMuchShorterCacheUpdate(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "01234567abcdefghijk";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    testAssert(ctx._refs.len == 2);
    testAssert(ctx._refs.items[0].idx == 8);
    testAssert(ctx._refs.items[0].line == 0);
    testAssert(ctx._refs.items[0].col == 8);
    testAssert(ctx._refs.items[1].idx == 16);
    testAssert(ctx._refs.items[1].line == 0);
    testAssert(ctx._refs.items[1].col == 16);

    _ctxReplace(&ctx, 1, 6, NULL, 0);

    testAssert(ctx._refs.len == 1);
    testAssert(ctx._refs.items[0].idx == 11);
    testAssert(ctx._refs.items[0].line == 0);
    testAssert(ctx._refs.items[0].col == 11);

    ctxDestroy(&ctx);
}

void test_ctxReplaceEmptyCacheUpdate(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "01234567abc";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    testAssert(ctx._refs.len == 1);
    testAssert(ctx._refs.items[0].idx == 8);
    testAssert(ctx._refs.items[0].line == 0);
    testAssert(ctx._refs.items[0].col == 8);

    _ctxReplace(&ctx, 2, 5, NULL, 0);

    testAssert(ctx._refs.len == 1);
    testAssert(ctx._refs.items[0].idx == 5);
    testAssert(ctx._refs.items[0].line == 0);
    testAssert(ctx._refs.items[0].col == 5);

    ctxDestroy(&ctx);
}

void test_ctxReplaceLongerCacheUpdate(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "01234567abc";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    testAssert(ctx._refs.len == 1);
    testAssert(ctx._refs.items[0].idx == 8);
    testAssert(ctx._refs.items[0].line == 0);
    testAssert(ctx._refs.items[0].col == 8);

    _ctxReplace(&ctx, 2, 5, (Utf8Ch *)"rplx", 4);

    testAssert(ctx._refs.len == 1);
    testAssert(ctx._refs.items[0].idx == 9);
    testAssert(ctx._refs.items[0].line == 0);
    testAssert(ctx._refs.items[0].col == 9);

    ctxDestroy(&ctx);
}

void test_ctxReplaceMuchLongerCacheUpdate(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "01234567abcdefghijk";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    testAssert(ctx._refs.len == 2);
    testAssert(ctx._refs.items[0].idx == 8);
    testAssert(ctx._refs.items[0].line == 0);
    testAssert(ctx._refs.items[0].col == 8);
    testAssert(ctx._refs.items[1].idx == 16);
    testAssert(ctx._refs.items[1].line == 0);
    testAssert(ctx._refs.items[1].col == 16);

    _ctxReplace(&ctx, 2, 2, (Utf8Ch *)"replace", 7);

    testAssert(ctx._refs.len == 3);
    testAssert(ctx._refs.items[0].idx == 8);
    testAssert(ctx._refs.items[0].line == 0);
    testAssert(ctx._refs.items[0].col == 8);
    testAssert(ctx._refs.items[1].idx == 15);
    testAssert(ctx._refs.items[1].line == 0);
    testAssert(ctx._refs.items[1].col == 15);
    testAssert(ctx._refs.items[2].idx == 23);
    testAssert(ctx._refs.items[2].line == 0);
    testAssert(ctx._refs.items[2].col == 23);

    ctxDestroy(&ctx);
}

void test_ctxReplaceCacheUpdateWithTabShorter(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "01234567abcdefgh123\t5678ijkl";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    _ctxReplace(&ctx, 2, 5, (Utf8Ch *)"rp", 2);

    testAssert(ctx._refs.len == 3);
    testAssert(ctx._refs.items[0].idx == 7);
    testAssert(ctx._refs.items[0].line == 0);
    testAssert(ctx._refs.items[0].col == 7);
    testAssert(ctx._refs.items[1].idx == 15);
    testAssert(ctx._refs.items[1].line == 0);
    testAssert(ctx._refs.items[1].col == 15);
    testAssert(ctx._refs.items[2].idx == 23);
    testAssert(ctx._refs.items[2].line == 0);
    testAssert(ctx._refs.items[2].col == 28);

    ctxDestroy(&ctx);
}

void test_ctxReplaceCacheUpdateWithTabMuchShorter(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "01234567abcdefgh123\t5678ijkl";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    _ctxReplace(&ctx, 1, 6, NULL, 0);

    testAssert(ctx._refs.len == 2);
    testAssert(ctx._refs.items[0].idx == 11);
    testAssert(ctx._refs.items[0].line == 0);
    testAssert(ctx._refs.items[0].col == 11);
    testAssert(ctx._refs.items[1].idx == 19);
    testAssert(ctx._refs.items[1].line == 0);
    testAssert(ctx._refs.items[1].col == 20);

    ctxDestroy(&ctx);
}

void test_ctxReplaceCacheUpdateWithTabLonger(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "01234567abcdefgh123\t5678ijkl";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    _ctxReplace(&ctx, 2, 5, (Utf8Ch *)"rplx", 4);

    testAssert(ctx._refs.len == 3);
    testAssert(ctx._refs.items[0].idx == 9);
    testAssert(ctx._refs.items[0].line == 0);
    testAssert(ctx._refs.items[0].col == 9);
    testAssert(ctx._refs.items[1].idx == 17);
    testAssert(ctx._refs.items[1].line == 0);
    testAssert(ctx._refs.items[1].col == 17);
    testAssert(ctx._refs.items[2].idx == 25);
    testAssert(ctx._refs.items[2].line == 0);
    testAssert(ctx._refs.items[2].col == 28);

    ctxDestroy(&ctx);
}

void test_ctxReplaceCacheUpdateWithTabMuchLonger(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "01234567abcdefgh123\t5678ijkl";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    _ctxReplace(&ctx, 2, 2, (Utf8Ch *)"replace", 7);

    testAssert(ctx._refs.len == 4);
    testAssert(ctx._refs.items[0].idx == 8);
    testAssert(ctx._refs.items[0].line == 0);
    testAssert(ctx._refs.items[0].col == 8);
    testAssert(ctx._refs.items[1].idx == 15);
    testAssert(ctx._refs.items[1].line == 0);
    testAssert(ctx._refs.items[1].col == 15);
    testAssert(ctx._refs.items[2].idx == 23);
    testAssert(ctx._refs.items[2].line == 0);
    testAssert(ctx._refs.items[2].col == 23);
    testAssert(ctx._refs.items[3].idx == 31);
    testAssert(ctx._refs.items[3].line == 0);
    testAssert(ctx._refs.items[3].col == 36);

    ctxDestroy(&ctx);
}

void test_ctxReplaceCacheUpdateAtMiddle(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "01234567abcdefgh123\t5678ijkl";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    _ctxReplace(&ctx, 12, 12, (Utf8Ch *)"replace", 7);

    testAssert(ctx._refs.len == 4);
    testAssert(ctx._refs.items[0].idx == 8);
    testAssert(ctx._refs.items[0].line == 0);
    testAssert(ctx._refs.items[0].col == 8);
    testAssert(ctx._refs.items[1].idx == 16);
    testAssert(ctx._refs.items[1].line == 0);
    testAssert(ctx._refs.items[1].col == 16);
    testAssert(ctx._refs.items[2].idx == 23);
    testAssert(ctx._refs.items[2].line == 0);
    testAssert(ctx._refs.items[2].col == 23);
    testAssert(ctx._refs.items[3].idx == 31);
    testAssert(ctx._refs.items[3].line == 0);
    testAssert(ctx._refs.items[3].col == 36);

    ctxDestroy(&ctx);
}

void test_ctxReplaceCacheUpdateWithLineFeed(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "01234567abc\nefgh1234";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    _ctxReplace(&ctx, 2, 5, (Utf8Ch *)"r\np", 3);

    testAssert(ctx._refs.len == 2);
    testAssert(ctx._refs.items[0].idx == 8);
    testAssert(ctx._refs.items[0].line == 1);
    testAssert(ctx._refs.items[0].col == 4);
    testAssert(ctx._refs.items[1].idx == 16);
    testAssert(ctx._refs.items[1].line == 2);
    testAssert(ctx._refs.items[1].col == 4);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxReplaceSameLen),
    testMake(test_ctxReplaceShorter),
    testMake(test_ctxReplaceEmpty),
    testMake(test_ctxReplaceLonger),

    testMake(test_ctxReplaceAtBeginning),
    testMake(test_ctxReplaceAtEnd),

    testMake(test_ctxReplaceSameLenWCursors),
    testMake(test_ctxReplaceShorterWCursors),
    testMake(test_ctxReplaceEmptyWCursors),
    testMake(test_ctxReplaceLongerWCursors),

    testMake(test_ctxReplaceWSingleCursor),

    testMake(test_ctxReplaceSameLenWSelCursors),
    testMake(test_ctxReplaceShorterWSelCursors),
    testMake(test_ctxReplaceEmptyWSelCursors),
    testMake(test_ctxReplaceLongerWSelCursors),

    testMake(test_ctxReplaceSameLenWSelections),
    testMake(test_ctxReplaceShorterWSelections),
    testMake(test_ctxReplaceEmptyWSelections),
    testMake(test_ctxReplaceEmptyJoinSelections),
    testMake(test_ctxReplaceLongerWSelections),

    testMake(test_ctxReplaceSameLenCacheUpdate),
    testMake(test_ctxReplaceShorterCacheUpdate),
    testMake(test_ctxReplaceMuchShorterCacheUpdate),
    testMake(test_ctxReplaceEmptyCacheUpdate),
    testMake(test_ctxReplaceLongerCacheUpdate),
    testMake(test_ctxReplaceMuchLongerCacheUpdate),

    testMake(test_ctxReplaceCacheUpdateWithTabShorter),
    testMake(test_ctxReplaceCacheUpdateWithTabMuchShorter),
    testMake(test_ctxReplaceCacheUpdateWithTabLonger),
    testMake(test_ctxReplaceCacheUpdateWithTabMuchLonger),
    testMake(test_ctxReplaceCacheUpdateAtMiddle),

    testMake(test_ctxReplaceCacheUpdateWithLineFeed)
)
