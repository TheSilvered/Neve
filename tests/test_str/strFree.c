#include "nv_test.h"
#include "nv_string.h"

void test_strFreeNullPtr(void) {
    strFree(NULL);
}

void test_strFreeEmpty(void) {
    Str *str = strNew(0);
    testAssertRequire(str != NULL);
    strFree(str);
}

void test_strFreeFull(void) {
    Str *str = strNewFromC("a");
    testAssertRequire(str != NULL);
    strFree(str);
}

testList(
    testMake(test_strFreeNullPtr),
    testMake(test_strFreeEmpty),
    testMake(test_strFreeFull)
)
