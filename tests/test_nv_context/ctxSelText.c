#include "nv_test.h"
#include "nv_context.c"

void test_ctxSelText(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab cde f g";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    ctxCurAdd(&ctx, 0);
    ctxCurAdd(&ctx, 3);
    ctxCurAdd(&ctx, 7);
    ctxCurAdd(&ctx, 9);

    ctxSelBegin(&ctx);
    ctxCurMoveToNextWordEnd(&ctx);
    ctxSelEnd(&ctx);

    Str *res = ctxSelText(&ctx);
    testAssert(strcmp(strAsC(res), "ab\ncde\nf\ng\n"));

    strFree(res);
    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxSelText)
)

