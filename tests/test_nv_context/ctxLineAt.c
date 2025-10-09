#include "nv_context.h"
#include "nv_test.h"
#define lineRefBlockShift_ 3
#include "nv_context.c"

void test_ctxLineAtNoLineRef(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncd\ne";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    size_t lineNo, startIdx;
    ctxLineAt_(&ctx, 0, &lineNo, &startIdx);

    testAssert(lineNo == 0);
    testAssert(startIdx == 0);

    ctxLineAt_(&ctx, 1, &lineNo, &startIdx);

    testAssert(lineNo == 0);
    testAssert(startIdx == 0);

    ctxLineAt_(&ctx, 2, &lineNo, &startIdx);

    testAssert(lineNo == 0);
    testAssert(startIdx == 0);

    ctxLineAt_(&ctx, 3, &lineNo, &startIdx);

    testAssert(lineNo == 1);
    testAssert(startIdx == 3);

    ctxLineAt_(&ctx, 4, &lineNo, &startIdx);

    testAssert(lineNo == 1);
    testAssert(startIdx == 3);

    ctxLineAt_(&ctx, 5, &lineNo, &startIdx);

    testAssert(lineNo == 1);
    testAssert(startIdx == 3);

    ctxLineAt_(&ctx, 6, &lineNo, &startIdx);

    testAssert(lineNo == 2);
    testAssert(startIdx == 6);

    ctxLineAt_(&ctx, 7, &lineNo, &startIdx);

    testAssert(lineNo == 2);
    testAssert(startIdx == 6);

    ctxDestroy(&ctx);
}

void test_ctxLineAtWithLineRef(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "ab\ncd\ne";
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));

    size_t lineNo, startIdx;
    ctxLineAt_(&ctx, 0, &lineNo, &startIdx);

    testAssert(lineNo == 0);
    testAssert(startIdx == 0);

    ctxLineAt_(&ctx, 2, &lineNo, &startIdx);

    testAssert(lineNo == 0);
    testAssert(startIdx == 0);

    ctxLineAt_(&ctx, 3, &lineNo, &startIdx);

    testAssert(lineNo == 1);
    testAssert(startIdx == 3);

    ctxLineAt_(&ctx, 7, &lineNo, &startIdx);

    testAssert(lineNo == 2);
    testAssert(startIdx == 6);

    ctxLineAt_(&ctx, 8, &lineNo, &startIdx);

    testAssert(lineNo == 2);
    testAssert(startIdx == 6);

    ctxLineAt_(&ctx, 9, &lineNo, &startIdx);

    testAssert(lineNo == 2);
    testAssert(startIdx == 6);

    ctxLineAt_(&ctx, 10, &lineNo, &startIdx);

    testAssert(lineNo == 3);
    testAssert(startIdx == 10);

    ctxLineAt_(&ctx, 11, &lineNo, &startIdx);

    testAssert(lineNo == 3);
    testAssert(startIdx == 10);

    ctxLineAt_(&ctx, 12, &lineNo, &startIdx);

    testAssert(lineNo == 3);
    testAssert(startIdx == 10);

    ctxLineAt_(&ctx, 13, &lineNo, &startIdx);

    testAssert(lineNo == 4);
    testAssert(startIdx == 13);

    ctxLineAt_(&ctx, 14, &lineNo, &startIdx);

    testAssert(lineNo == 4);
    testAssert(startIdx == 13);

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxLineAtNoLineRef),
    testMake(test_ctxLineAtWithLineRef)
)

