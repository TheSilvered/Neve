#include "nv_test.h"
#include "nv_context.c"

void test_ctxCurMoveToNextParagraph(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncd\n\nef\n\n\n\n\ngh";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    ctxCurAdd(&ctx, 0);
    ctxCurMoveToNextParagraph(&ctx);
    testAssert(ctx.cursors.items[0].idx == 6);
    ctxCurMoveToNextParagraph(&ctx);
    testAssert(ctx.cursors.items[0].idx == 10);
    ctxCurMoveToNextParagraph(&ctx);
    testAssert(ctx.cursors.items[0].idx == 16);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxCurMoveToNextParagraph)
)
