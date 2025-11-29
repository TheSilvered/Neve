#include "nv_test.h"
#include "nv_array.h"

void test_arrAppend(void) {
    Arr(int) arr = { 0 };

    arrAppend(&arr, 1);
    arrAppend(&arr, 2);
    arrAppend(&arr, 3);

    testAssert(arr.len == 3);
    testAssert(arr.items[0] == 1);
    testAssert(arr.items[1] == 2);
    testAssert(arr.items[2] == 3);
    testAssert(arr.cap >= arr.len);

    arrClear(&arr);
}

testList(
    testMake(test_arrAppend)
)
