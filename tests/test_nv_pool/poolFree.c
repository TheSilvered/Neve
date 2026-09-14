#include "nv_test.h"
#include "nv_pool.h"

void test_poolFreeFromEmpty(void) {
    Pool p = poolMake(48);
    poolFree(&p, NULL);
}

void test_poolFreeFromFirstPage(void) {
    Pool p = poolMake(48);
    void *block = poolAlloc(&p);
    poolFree(&p, block);
    poolDestroy(&p);
}

testList(
    testMakeFail(test_poolFreeFromEmpty),
    testMake(test_poolFreeFromFirstPage)
)
