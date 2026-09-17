#include "nv_test.h"
#include "nv_key_binds.h"

void test_bindRemove(void) {
    bindInit();
    bindAdd(BindMap_Normal, BindSeq('a'), (KeyBind){ 0 });
    testAssert(bindExists(BindMap_Normal, BindSeq('a')));
    bindRemove(BindMap_Normal, BindSeq('a'));
    testAssert(!bindExists(BindMap_Normal, BindSeq('a')));
    bindQuit();
}

testList(
    testMake(test_bindRemove)
)
