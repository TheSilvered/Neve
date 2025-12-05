#include "nv_test.h"
#include "nv_buffer.c"

void test_bufNewEmpty(void) {
    BufMap map;
    bufMapInit(&map);

    BufHandle hd1 = bufInitEmpty(&map);
    testAssert(hd1 != bufInvalidHandle);
    BufHandle hd2 = bufInitEmpty(&map);
    testAssert(hd2 != bufInvalidHandle);
    testAssert(hd1 != hd2);

    for (int i = 0; i < 100; i++) {
        BufHandle hd = bufInitEmpty(&map);
        testAssert(hd != bufInvalidHandle);
    }

    bufMapDestroy(&map);
}

testList(
    testMake(test_bufNewEmpty)
)
