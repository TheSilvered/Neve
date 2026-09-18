#include "nv_test.h"
#include "nv_key_binds.h"

typedef BindMatchResult BMR;

void test_bindMatchNotFound(void) {
    bindInit();

    KeyBind bind = { 0 };
    bindAdd(BindMap_Normal, BindSeq('x'), bind);
    bindAdd(BindMap_Normal, BindSeq('x', 'y'), bind);
    bindAdd(BindMap_Normal, BindSeq('x', 'z'), bind);
    bindAdd(BindMap_Normal, BindSeq('w'), bind);

    BMR result;
    result = bindMatch(BindMap_Normal, BindSeq('a'), &bind);
    testAssert(result == BindMatch_NotFound);
    result = bindMatch(BindMap_Normal, BindSeq('x', 'w'), &bind);
    testAssert(result == BindMatch_NotFound);
    result = bindMatch(BindMap_Normal, BindSeq('x', 'y', 'z'), &bind);
    testAssert(result == BindMatch_NotFound);

    bindQuit();
}

void test_bindMatchIncomplete(void) {
    bindInit();

    KeyBind bind = { 0 };
    bindAdd(BindMap_Normal, BindSeq('x', 'y', 'z'), bind);
    bindAdd(BindMap_Normal, BindSeq('a', BindAny, 'c'), bind);

    BMR result;
    result = bindMatch(BindMap_Normal, BindSeq('x', 'y'), &bind);
    testAssert(result == BindMatch_Incomplete);
    result = bindMatch(BindMap_Normal, BindSeq('a', 'b'), &bind);
    testAssert(result == BindMatch_Incomplete);

    bindQuit();
}

void test_bindMatchFound(void) {
    bindInit();

    KeyBind bind = { 0 };
    bindAdd(BindMap_Normal, BindSeq('x', 'y', 'z'), bind);
    bindAdd(BindMap_Normal, BindSeq('a', BindAny, 'c'), bind);

    BMR result;
    result = bindMatch(BindMap_Normal, BindSeq('x', 'y', 'z'), &bind);
    testAssert(result == BindMatch_Found);
    result = bindMatch(BindMap_Normal, BindSeq('a', 'b', 'c'), &bind);
    testAssert(result == BindMatch_Found);

    bindQuit();
}

void test_bindMatchPartial(void) {
    bindInit();

    KeyBind bind = { 0 };
    bindAdd(BindMap_Normal, BindSeq('x', 'y'), bind);
    bindAdd(BindMap_Normal, BindSeq('x', 'y', 'z'), bind);
    bindAdd(BindMap_Normal, BindSeq('a', 'b'), bind);
    bindAdd(BindMap_Normal, BindSeq('a', BindAny, 'c'), bind);
    bindAdd(BindMap_Normal, BindSeq('i', BindAny, 'k'), bind);
    bindAdd(BindMap_Normal, BindSeq('i', 'j', 'k', 'l'), bind);

    BMR result;
    result = bindMatch(BindMap_Normal, BindSeq('x', 'y'), &bind);
    testAssert(result == BindMatch_Partial);
    result = bindMatch(BindMap_Normal, BindSeq('a', 'b'), &bind);
    testAssert(result == BindMatch_Partial);
    result = bindMatch(BindMap_Normal, BindSeq('i', 'j', 'k'), &bind);
    testAssert(result == BindMatch_Partial);

    bindQuit();
}

testList(
    testMake(test_bindMatchNotFound),
    testMake(test_bindMatchIncomplete),
    testMake(test_bindMatchFound),
    testMake(test_bindMatchPartial),
)
