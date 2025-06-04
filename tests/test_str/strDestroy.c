#include "nv_test.h"
#include "nv_str.h"

void test_strDestroyNullPtr(void) {
    strDestroy(NULL);
}

void test_strDestroyEmpty(void) {
    Str str;
    testAssertRequire(strInit(&str, 0));
    strDestroy(&str);
}

void test_strDestroyFull(void) {
    Str str;
    testAssertRequire(strInitFromC(&str, "a"));
    strDestroy(&str);
}

testList(
    testMake(test_strDestroyNullPtr),
    testMake(test_strDestroyEmpty),
    testMake(test_strDestroyFull)
)
