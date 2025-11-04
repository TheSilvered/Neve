#include "nv_test.h"
#define lineRefMaxGap_ 4
#include "nv_context.c"

void test_ctxAppendFromEmptyNoLines(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "abcd";

    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));
    testAssertWith(ctx._refs.len == 1) {
        testAssert(ctx._refs.items[0].line == 0);
    }
    testAssert(ctx._buf.len == chArrLen(s));
    testAssert(ctx._buf.gapIdx == chArrLen(s));
    testAssert(ctx._buf.bytes[0] == 'a');
    testAssert(ctx._buf.bytes[1] == 'b');
    testAssert(ctx._buf.bytes[2] == 'c');
    testAssert(ctx._buf.bytes[3] == 'd');

    ctxDestroy(&ctx);
}

void test_ctxAppendFromEmptyWithLines(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "a\nc\n";

    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));
    testAssertWith(ctx._refs.len == 1) {
        testAssert(ctx._refs.items[0].line == 2);
    }

    testAssert(ctx._buf.len == chArrLen(s));
    testAssert(ctx._buf.gapIdx == chArrLen(s));
    testAssert(ctx._buf.bytes[0] == 'a');
    testAssert(ctx._buf.bytes[1] == '\n');
    testAssert(ctx._buf.bytes[2] == 'c');
    testAssert(ctx._buf.bytes[3] == '\n');

    ctxDestroy(&ctx);
}

void test_ctxAppendFromEmptyWithCRLFLines(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "a\r\nc\r\n";

    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));
    testAssertWith(ctx._refs.len == 1) {
        testAssert(ctx._refs.items[0].line == 2);
    }

    testAssert(ctx._buf.len == chArrLen(s) - 2);
    testAssert(ctx._buf.gapIdx == chArrLen(s) - 2);
    testAssert(ctx._buf.bytes[0] == 'a');
    testAssert(ctx._buf.bytes[1] == '\n');
    testAssert(ctx._buf.bytes[2] == 'c');
    testAssert(ctx._buf.bytes[3] == '\n');

    ctxDestroy(&ctx);
}
void test_ctxAppendFromFullNoLines(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "a\nc\n";
    const char s2[] = "abcd";

    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));
    ctxAppend(&ctx, (UcdCh8 *)s2, chArrLen(s2));
    testAssertWith(ctx._refs.len == 2) {
        testAssert(ctx._refs.items[0].line == 2);
        testAssert(ctx._refs.items[1].line == 2);
    }

    testAssert(ctx._buf.len == chArrLen(s) + chArrLen(s2));
    testAssert(ctx._buf.gapIdx == chArrLen(s) + chArrLen(s2));
    testAssert(ctx._buf.bytes[0] == 'a');
    testAssert(ctx._buf.bytes[1] == '\n');
    testAssert(ctx._buf.bytes[2] == 'c');
    testAssert(ctx._buf.bytes[3] == '\n');
    testAssert(ctx._buf.bytes[4] == 'a');
    testAssert(ctx._buf.bytes[5] == 'b');
    testAssert(ctx._buf.bytes[6] == 'c');
    testAssert(ctx._buf.bytes[7] == 'd');

    ctxDestroy(&ctx);
}

void test_ctxAppendFromFullWithLines(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "a\nc\n";

    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));
    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));
    testAssertWith(ctx._refs.len == 2) {
        testAssert(ctx._refs.items[0].line == 2);
        testAssert(ctx._refs.items[1].line == 4);
    }

    testAssert(ctx._buf.len == 2 * chArrLen(s));
    testAssert(ctx._buf.gapIdx == 2 * chArrLen(s));
    testAssert(ctx._buf.bytes[0] == 'a');
    testAssert(ctx._buf.bytes[1] == '\n');
    testAssert(ctx._buf.bytes[2] == 'c');
    testAssert(ctx._buf.bytes[3] == '\n');
    testAssert(ctx._buf.bytes[4] == 'a');
    testAssert(ctx._buf.bytes[5] == '\n');
    testAssert(ctx._buf.bytes[6] == 'c');
    testAssert(ctx._buf.bytes[7] == '\n');

    ctxDestroy(&ctx);
}

void test_ctxAppendFromHalfFullNoLines(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "a\n";
    const char s2[] = "abcd";

    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));
    ctxAppend(&ctx, (UcdCh8 *)s2, chArrLen(s2));
    testAssertWith(ctx._refs.len == 1) {
        testAssert(ctx._refs.items[0].line == 1);
    }

    testAssert(ctx._buf.len == chArrLen(s) + chArrLen(s2));
    testAssert(ctx._buf.gapIdx == chArrLen(s) + chArrLen(s2));
    testAssert(ctx._buf.bytes[0] == 'a');
    testAssert(ctx._buf.bytes[1] == '\n');
    testAssert(ctx._buf.bytes[2] == 'a');
    testAssert(ctx._buf.bytes[3] == 'b');
    testAssert(ctx._buf.bytes[4] == 'c');
    testAssert(ctx._buf.bytes[5] == 'd');

    ctxDestroy(&ctx);
}

void test_ctxAppendFromHalfFullWithLines(void) {
    Ctx ctx;
    ctxInit(&ctx, true);
    const char s[] = "a\n";
    const char s2[] = "a\nc\n";

    ctxAppend(&ctx, (UcdCh8 *)s, chArrLen(s));
    ctxAppend(&ctx, (UcdCh8 *)s2, chArrLen(s2));
    testAssertWith(ctx._refs.len == 1) {
        testAssert(ctx._refs.items[0].line == 2);
    }
    testAssert(ctx._buf.len == chArrLen(s) + chArrLen(s2));
    testAssert(ctx._buf.gapIdx == chArrLen(s) + chArrLen(s2));
    testAssert(ctx._buf.bytes[0] == 'a');
    testAssert(ctx._buf.bytes[1] == '\n');
    testAssert(ctx._buf.bytes[2] == 'a');
    testAssert(ctx._buf.bytes[3] == '\n');
    testAssert(ctx._buf.bytes[4] == 'c');
    testAssert(ctx._buf.bytes[5] == '\n');

    ctxDestroy(&ctx);
}

testList(
    testMake(test_ctxAppendFromEmptyNoLines),
    testMake(test_ctxAppendFromEmptyWithLines),
    testMake(test_ctxAppendFromEmptyWithCRLFLines),
    testMake(test_ctxAppendFromFullNoLines),
    testMake(test_ctxAppendFromFullWithLines),
    testMake(test_ctxAppendFromHalfFullNoLines),
    testMake(test_ctxAppendFromHalfFullWithLines)
)
