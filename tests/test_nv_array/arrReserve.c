#include "nv_test.h"
#include "nv_array.h"

void test_arrReserveFromEmpty(void) {
    Arr(int) arr = { 0 };

    testAssert(arr.len == 0);
    testAssert(arr.cap == 0);

    arrReserve(&arr, 10);

    testAssert(arr.len == 0);
    testAssert(arr.cap >= 10);

    arrClear(&arr);
}

void test_arrReserveFromFull(void) {
    Arr(int) arr = { 0 };

    arrAppend(&arr, 1);
    arrAppend(&arr, 2);
    arrAppend(&arr, 3);
    testAssert(arr.cap >= 3);

    arrReserve(&arr, 10);

    testAssert(arr.len == 3);
    testAssert(arr.cap >= 10);

    arrClear(&arr);
}

testList(
    testMake(test_arrReserveFromEmpty),
    testMake(test_arrReserveFromFull),
)
