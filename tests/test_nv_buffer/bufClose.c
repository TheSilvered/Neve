#include "nv_test.h"
#include "nv_buffer.c"

void test_bufCloseSingle(void) {
    BufMap map;
    bufMapInit(&map);

    BufHandle hd = bufInitEmpty(&map);
    testAssertRequire(hd != bufInvalidHandle);
    bufClose(&map, hd);

    testAssert(map.len == 0);

    bufMapDestroy(&map);
}

void test_bufCloseMultiple(void) {
#define arrSize 100
    BufMap map;
    bufMapInit(&map);

    BufHandle handles[arrSize];
    Buf *bufs[arrSize];

    for (int i = 0; i < arrSize; i++) {
        handles[i] = bufInitEmpty(&map);
        testAssertRequire(handles[i] != bufInvalidHandle);
        bufs[i] = bufRef(&map, handles[i]);
    }

    for (int i = 0; i < arrSize; i++) {
        bufClose(&map, handles[arrSize - i - 1]);
        for (int j = 0; j < arrSize - i - 1; j++) {
            testAssert(bufRef(&map, handles[j]) == bufs[j]);
        }
        testAssert(map.len == arrSize - i - 1);
    }

    bufMapDestroy(&map);
#undef arrSize
}

testList(
    testMake(test_bufCloseSingle),
    testMake(test_bufCloseMultiple)
)
