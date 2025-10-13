#include "nv_test.h"
#include "nv_context.c"

void test_ctxCursorAdd(void) {
    Ctx ctx;
    ctxInit(&ctx, true);

    ctxCursorAdd_(&ctx, 3);
    testAssert(ctx.m_cursors.len == 1);
    testAssert(ctx.m_cursors.items[0] = 3);

    ctxCursorAdd_(&ctx, 5);
    testAssert(ctx.m_cursors.len == 2);
    testAssert(ctx.m_cursors.items[0] = 3);
    testAssert(ctx.m_cursors.items[1] = 5);

    ctxCursorAdd_(&ctx, 2);
    testAssert(ctx.m_cursors.len == 3);
    testAssert(ctx.m_cursors.items[0] = 2);
    testAssert(ctx.m_cursors.items[1] = 3);
    testAssert(ctx.m_cursors.items[2] = 5);

    ctxCursorAdd_(&ctx, 4);
    testAssert(ctx.m_cursors.len == 4);
    testAssert(ctx.m_cursors.items[0] = 2);
    testAssert(ctx.m_cursors.items[1] = 3);
    testAssert(ctx.m_cursors.items[2] = 4);
    testAssert(ctx.m_cursors.items[3] = 5);

    ctxCursorAdd_(&ctx, 4);
    testAssert(ctx.m_cursors.len == 4);
    testAssert(ctx.m_cursors.items[0] = 2);
    testAssert(ctx.m_cursors.items[1] = 3);
    testAssert(ctx.m_cursors.items[2] = 4);
    testAssert(ctx.m_cursors.items[3] = 5);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxCursorAdd)
)

