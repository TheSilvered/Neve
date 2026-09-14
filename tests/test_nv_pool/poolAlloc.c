#include "nv_test.h"
#include "nv_pool.h"

void test_poolAllocFromEmpty(void) {
    const size_t blockSize = 48;
    Pool p = poolMake(blockSize);
    void *block = poolAlloc(&p);
    testAssert(memIsInAlloc(block, blockSize));
    poolDestroy(&p);
}

void test_poolAllocFromFullSamePage(void) {
    const size_t blockSize = 48;
    Pool p = poolMake(blockSize);
    void *block0 = poolAlloc(&p);
    void *block = poolAlloc(&p);
    testAssert(memIsInAlloc(block, blockSize));
    testAssert((uint8_t *)block0 - blockSize == (uint8_t *)block);
    poolDestroy(&p);
}

void test_poolAllocFromFullChangePage(void) {
    const size_t blockSize = 48;
    Pool p = poolMakeEx(blockSize, blockSize); // 1 block per page
    void *block0 = poolAlloc(&p);
    PoolPage *page0 = p.pages;
    testAssert(page0->freeCount == 0);
    void *block = poolAlloc(&p);
    PoolPage *page1 = p.pages;
    testAssert(memIsInAlloc(block, blockSize));
    testAssert((uint8_t *)block0 - blockSize != (uint8_t *)block);
    testAssert(page0 != page1);
    poolDestroy(&p);
}

void test_poolAllocFillHoleSamePage(void) {
    const size_t blockSize = 48;
    Pool p = poolMakeEx(blockSize, blockSize * 2); // 2 blocks per page
    void *b0 = poolAlloc(&p);
    void *b1 = poolAlloc(&p); (void)b1;
    PoolPage *page0 = p.pages;
    testAssert(page0->freeCount == 0);
    poolFree(&p, b0);
    void *b2 = poolAlloc(&p);
    testAssert(memIsInAlloc(b2, blockSize));
    testAssert(b0 == b2);
    testAssert(p.pages == page0);
    poolDestroy(&p);
}

void test_poolAllocFillHoleOldPage(void) {
    const size_t blockSize = 48;
    Pool p = poolMakeEx(blockSize, blockSize * 2); // 2 blocks per page
    // Fill the first page
    void *p0b0 = poolAlloc(&p);
    void *p0b1 = poolAlloc(&p);
    PoolPage *page0 = p.pages;
    // Fill the second page
    void *p1b0 = poolAlloc(&p);
    void *p1b1 = poolAlloc(&p);
    PoolPage *page1 = p.pages;
    testAssert(page0 != page1);
    poolFree(&p, p0b0);
    void *p0b3 = poolAlloc(&p);
    testAssert(memIsInAlloc(p0b3, blockSize));
    testAssert(p0b3 == p0b0);
    testAssert(p.pages == page0);
    poolDestroy(&p);
}

testList(
    testMake(test_poolAllocFromEmpty),
    testMake(test_poolAllocFromFullSamePage),
    testMake(test_poolAllocFromFullChangePage),
    testMake(test_poolAllocFillHoleSamePage),
    testMake(test_poolAllocFillHoleOldPage)
)
