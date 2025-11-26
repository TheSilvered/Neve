#include "nv_test.h"
#include "nv_escapes.h"
#include "nv_context.c"

bool eqStrViewCStr(StrView sv, const char *cStr) {
    return strncmp((char *)sv.buf, cStr, sv.len) == 0;
}

void test_ctxInsertFromEmpty(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    ctxCurAdd(&ctx, 0);

    ctxInsert(&ctx, sLen("abcd"));

    testAssert(eqStrViewCStr(ctxGetContent(&ctx), "abcd"));

    ctxDestroy(&ctx);
}

void test_ctxInsertFromEmptyActiveSel(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    ctxAppend(&ctx, sLen("abcd"));

    ctxCurAdd(&ctx, 2);
    ctxSelBegin(&ctx);

    ctxInsert(&ctx, sLen("xyz"));

    testAssert(eqStrViewCStr(ctxGetContent(&ctx), "abcd"));

    ctxDestroy(&ctx);
}

void test_ctxInsertMultipleCursors(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    ctxAppend(&ctx, sLen("abcd"));

    ctxCurAdd(&ctx, 1);
    ctxCurAdd(&ctx, 3);

    ctxInsert(&ctx, sLen("xyz"));
    testAssert(eqStrViewCStr(ctxGetContent(&ctx), "axyzbcxyzd"));

    ctxInsert(&ctx, sLen("12\n34"));
    testAssert(eqStrViewCStr(ctxGetContent(&ctx), "axyz12bcxyz34d"));

    ctxInsert(&ctx, sLen("56\n78\n"));
    testAssert(eqStrViewCStr(ctxGetContent(&ctx), "axyz1256bcxyz3478d"));

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxInsertFromEmpty),
    testMake(test_ctxInsertFromEmptyActiveSel),
    testMake(test_ctxInsertMultipleCursors)
)
