#include "nv_test.h"
#include "nv_context.c"

void test_ctxCursorRemove(void) {
    Ctx ctx;
    ctxInit(&ctx, true);

    ctxCursorAdd_(&ctx, 2);
    ctxCursorAdd_(&ctx, 4);
    ctxCursorAdd_(&ctx, 8);
    ctxCursorAdd_(&ctx, 15);
    ctxCursorAdd_(&ctx, 16);

    ctxCursorRemove_(&ctx, 3);
    testAssert(ctx.m_cursors.len == 5);
    testAssert(ctx.m_cursors.items[0] == 2);
    testAssert(ctx.m_cursors.items[1] == 4);
    testAssert(ctx.m_cursors.items[2] == 8);
    testAssert(ctx.m_cursors.items[3] == 15);
    testAssert(ctx.m_cursors.items[4] == 16);

    ctxCursorRemove_(&ctx, 2);
    testAssert(ctx.m_cursors.len == 4);
    testAssert(ctx.m_cursors.items[0] == 4);
    testAssert(ctx.m_cursors.items[1] == 8);
    testAssert(ctx.m_cursors.items[2] == 15);
    testAssert(ctx.m_cursors.items[3] == 16);

    ctxCursorRemove_(&ctx, 8);
    testAssert(ctx.m_cursors.len == 3);
    testAssert(ctx.m_cursors.items[0] == 4);
    testAssert(ctx.m_cursors.items[1] == 15);
    testAssert(ctx.m_cursors.items[2] == 16);

    ctxCursorRemove_(&ctx, 16);
    testAssert(ctx.m_cursors.len == 2);
    testAssert(ctx.m_cursors.items[0] == 4);
    testAssert(ctx.m_cursors.items[1] == 15);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxCursorRemove)
)

