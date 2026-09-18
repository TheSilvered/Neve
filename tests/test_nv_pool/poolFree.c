#include "nv_test.h"
#include "nv_pool.h"

#ifdef NV_DEBUG

void test_poolFreeFromEmpty(void) {
    Pool p = poolMake(48);
    poolFree(&p, NULL);
}

void test_poolFreeDoubleFree(void) {
    Pool p = poolMake(48);
    void *block = poolAlloc(&p);
    poolFree(&p, block);
    poolFree(&p, block);
}

void test_poolFreeMisalignedFree(void) {
    Pool p = poolMake(48);
    uint8_t *block = poolAlloc(&p);
    poolFree(&p, block + 1);
}

#else

void test_poolFreeFromEmpty(void) {}
void test_poolFreeDoubleFree(void) {}
void test_poolFreeMisalignedFree(void) {}

#endif // !NV_DEBUG

void test_poolFreeFromFirstPage(void) {
    Pool p = poolMake(48);
    void *block = poolAlloc(&p);
    poolFree(&p, block);
    poolDestroy(&p);
}

void test_poolFreeFromFullPage(void) {
    const size_t blockSize = 48;
    Pool p = poolMakeEx(blockSize, blockSize);
    void *block = poolAlloc(&p);
    PoolPage *firstPage = p.pages;
    poolAlloc(&p);
    poolFree(&p, block);
    testAssert(p.freePages == firstPage);
    poolDestroy(&p);
}

void test_poolFreeFromPartialPage(void) {
    const size_t blockSize = 48;
    Pool p = poolMakeEx(blockSize, blockSize * 2);
    void *block0 = poolAlloc(&p);
    void *block1 = poolAlloc(&p);
    PoolPage *firstPage = p.pages;
    poolAlloc(&p);
    poolAlloc(&p);
    poolFree(&p, block0);
    poolFree(&p, block1);
    testAssert(p.freePages == firstPage);
    poolDestroy(&p);
}

testList(
    testMakeFail(test_poolFreeFromEmpty),
    testMakeFail(test_poolFreeDoubleFree),
    testMakeFail(test_poolFreeMisalignedFree),
    testMake(test_poolFreeFromFirstPage),
    testMake(test_poolFreeFromFullPage),
    testMake(test_poolFreeFromPartialPage),
)
