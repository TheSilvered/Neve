#include "nv_test.h"
#include "nv_context.c"

void test_ctxCurMoveToPrevParagraph(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncd\n\nef\n\n\n\n\ngh";
    ctxAppend(&ctx, (Utf8Ch *)s, chArrLen(s));

    ctxCurAdd(&ctx, 16);
    ctxCurMoveToPrevParagraph(&ctx);
    testAssert(ctx.cursors.items[0].idx == 13);
    ctxCurMoveToPrevParagraph(&ctx);
    testAssert(ctx.cursors.items[0].idx == 6);
    ctxCurMoveToPrevParagraph(&ctx);
    testAssert(ctx.cursors.items[0].idx == 0);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxCurMoveToPrevParagraph)
)
