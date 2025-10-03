// dummy test

#include "nv_test.h"
#include "nv_context.c"

void test_ctxInit(void) {
    Ctx ctx;
    ctxInit(&ctx, false);
}

testList(
    testMake(test_ctxInit)
)
