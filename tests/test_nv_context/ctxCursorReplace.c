#include "nv_test.h"
#include "nv_context.c"

void test_ctxCursorReplace(void) {
    Ctx ctx;
    ctxInit(&ctx, true);

    ctxCursorAdd_(&ctx, 2);
    ctxCursorAdd_(&ctx, 4);
    ctxCursorAdd_(&ctx, 6);

    ctxCursorReplace_(&ctx, 6, 7);
    testAssert(ctx.m_cursors.len == 3);
    testAssert(ctx.m_cursors.items[0] == 2);
    testAssert(ctx.m_cursors.items[1] == 4);
    testAssert(ctx.m_cursors.items[2] == 7);

    ctxCursorReplace_(&ctx, 2, 5);
    testAssert(ctx.m_cursors.len == 3);
    testAssert(ctx.m_cursors.items[0] == 4);
    testAssert(ctx.m_cursors.items[1] == 5);
    testAssert(ctx.m_cursors.items[2] == 7);

    ctxCursorReplace_(&ctx, 5, 6);
    testAssert(ctx.m_cursors.len == 3);
    testAssert(ctx.m_cursors.items[0] == 4);
    testAssert(ctx.m_cursors.items[1] == 6);
    testAssert(ctx.m_cursors.items[2] == 7);

    ctxCursorReplace_(&ctx, 1, 8);
    testAssert(ctx.m_cursors.len == 4);
    testAssert(ctx.m_cursors.items[0] == 4);
    testAssert(ctx.m_cursors.items[1] == 6);
    testAssert(ctx.m_cursors.items[2] == 7);
    testAssert(ctx.m_cursors.items[3] == 8);

    ctxCursorReplace_(&ctx, 6, 7);
    testAssert(ctx.m_cursors.len == 3);
    testAssert(ctx.m_cursors.items[0] == 4);
    testAssert(ctx.m_cursors.items[1] == 7);
    testAssert(ctx.m_cursors.items[2] == 8);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxCursorReplace)
)

