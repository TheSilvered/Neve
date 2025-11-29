#include "nv_test.h"
#include "nv_context.c"
#include "nv_escapes.h"

bool eqStrViewCStr(StrView sv, const char* cStr) {
	return strncmp((char*)sv.buf, cStr, sv.len) == 0;
}

void test_ctxRemoveBackFromStart(void) {
	Ctx ctx;
	ctxInit(&ctx, true);
	const char s[] = "abcd";
	ctxAppend(&ctx, sLen(s));

	ctxCurAdd(&ctx, 0);
	ctxRemoveBack(&ctx);
	testAssert(!ctxSelIsActive(&ctx));
	testAssert(eqStrViewCStr(ctxGetContent(&ctx), "abcd"));
	testAssert(ctx.cursors.items[0].idx == 0);

	ctxDestroy(&ctx);
}

void test_ctxRemoveBackFromMiddle(void) {
	Ctx ctx;
	ctxInit(&ctx, true);
	const char s[] = "abcd";
	ctxAppend(&ctx, sLen(s));

	ctxCurAdd(&ctx, 2);
	ctxRemoveBack(&ctx);
	testAssert(!ctxSelIsActive(&ctx));
	testAssert(eqStrViewCStr(ctxGetContent(&ctx), "acd"));
	testAssert(ctx.cursors.items[0].idx == 1);

	ctxDestroy(&ctx);
}

void test_ctxRemoveBackFromEnd(void) {
	Ctx ctx;
	ctxInit(&ctx, true);
	const char s[] = "abcd";
	ctxAppend(&ctx, sLen(s));

	ctxCurAdd(&ctx, 4);
	ctxRemoveBack(&ctx);
	testAssert(!ctxSelIsActive(&ctx));
	testAssert(eqStrViewCStr(ctxGetContent(&ctx), "abc"));
	testAssert(ctx.cursors.items[0].idx == 3);

	ctxDestroy(&ctx);
}

testList(
	testMake(test_ctxRemoveBackFromStart),
	testMake(test_ctxRemoveBackFromMiddle),
	testMake(test_ctxRemoveBackFromEnd)
)
