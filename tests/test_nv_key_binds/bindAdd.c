#include "nv_test.h"
#include "nv_key_binds.h"

static KeyBind emptyBind = { 0 };

void test_bindAddToRoot(void) {
    bindInit();
    bindAdd(BindMap_Normal, BindSeq('a'), emptyBind);
    bindAdd(BindMap_Normal, BindSeq('b'), emptyBind);
    bindAdd(BindMap_Normal, BindSeq(BindAny), emptyBind);
    testAssert(bindExists(BindMap_Normal, BindSeq('a')));
    testAssert(bindExists(BindMap_Normal, BindSeq('b')));
    testAssert(bindExists(BindMap_Normal, BindSeq(BindAny)));
    bindQuit();
}

void test_bindAddExtending(void) {
    bindInit();
    bindAdd(BindMap_Normal, BindSeq('a'), emptyBind);
    bindAdd(BindMap_Normal, BindSeq('a', 'b'), emptyBind);
    bindAdd(BindMap_Normal, BindSeq('a', 'b', 'c'), emptyBind);
    bindAdd(BindMap_Normal, BindSeq('a', 'c'), emptyBind);
    testAssert(bindExists(BindMap_Normal, BindSeq('a')));
    testAssert(bindExists(BindMap_Normal, BindSeq('a', 'b')));
    testAssert(bindExists(BindMap_Normal, BindSeq('a', 'b', 'c')));
    testAssert(bindExists(BindMap_Normal, BindSeq('a', 'c')));
    bindQuit();
}

testList(
    testMake(test_bindAddToRoot),
    testMake(test_bindAddExtending),
)
