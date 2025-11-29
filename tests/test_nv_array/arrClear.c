#include "nv_test.h"
#include "nv_array.h"

void test_arrDestroy(void) {
    Arr(int) arr = { 0 };
    arrAppend(&arr, 1);
    arrAppend(&arr, 2);
    arrAppend(&arr, 3);

    arrClear(&arr);

    testAssert(arr.items == NULL);
    testAssert(arr.len == 0);
    testAssert(arr.cap == 0);
}

testList(
    testMake(test_arrDestroy)
)
