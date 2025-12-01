#include "nv_test.h"
#include "nv_context.c"
#include "nv_escapes.h"

bool eqStrViewCStr(StrView sv, const char* cStr) {
    return strncmp((char*)sv.buf, cStr, sv.len) == 0;
}

void test_ctxRemoveFwdFromStart(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcd";
    ctxAppend(&ctx, sLen(s));

    ctxCurAdd(&ctx, 0);
    ctxRemoveFwd(&ctx);
    testAssert(!ctxSelIsActive(&ctx));
    testAssert(eqStrViewCStr(ctxGetContent(&ctx), "bcd"));
    testAssert(ctx.cursors.items[0].idx == 0);

    ctxDestroy(&ctx);
}

void test_ctxRemoveFwdFromMiddle(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcd";
    ctxAppend(&ctx, sLen(s));

    ctxCurAdd(&ctx, 2);
    ctxRemoveFwd(&ctx);
    testAssert(!ctxSelIsActive(&ctx));
    testAssert(eqStrViewCStr(ctxGetContent(&ctx), "abd"));
    testAssert(ctx.cursors.items[0].idx == 2);

    ctxDestroy(&ctx);
}

void test_ctxRemoveFwdFromEnd(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcd";
    ctxAppend(&ctx, sLen(s));

    ctxCurAdd(&ctx, 4);
    ctxRemoveFwd(&ctx);
    testAssert(!ctxSelIsActive(&ctx));
    testAssert(eqStrViewCStr(ctxGetContent(&ctx), "abcd"));
    testAssert(ctx.cursors.items[0].idx == 4);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxRemoveFwdFromStart),
    testMake(test_ctxRemoveFwdFromMiddle),
    testMake(test_ctxRemoveFwdFromEnd)
)
