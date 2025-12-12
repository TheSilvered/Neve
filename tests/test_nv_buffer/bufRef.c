#include "nv_test.h"
#include "nv_buffer.c"

void test_bufRefSingle(void) {
    BufMap map;
    bufMapInit(&map);

    BufHandle hd1 = bufInitEmpty(&map);
    Buf *buf = bufRef(&map, hd1);

    testAssert(buf != NULL);
    testAssert(memIsAlloc(buf));

    bufMapDestroy(&map);
}

void test_bufRefMany(void) {
#define arrSize 100
    BufHandle handles[arrSize] = { 0 };
    BufMap map;
    bufMapInit(&map);

    for (int i = 0; i < arrSize; i++) {
        handles[i] = bufInitEmpty(&map);
    }

    for (int i = 0; i < arrSize; i++) {
        Buf *buf = bufRef(&map, handles[i]);
        testAssert(buf != NULL);
        testAssert(memIsAlloc(buf));
    }

    bufMapDestroy(&map);
#undef arrSize
}

testList(
    testMake(test_bufRefSingle),
    testMake(test_bufRefMany)
)
