#include "nv_test.h"
#include "nv_array.h"

void test_arrResizeFromEmpty(void) {
    Arr(int) arr = { 0 };

    arrResize(&arr, 10);

    testAssert(arr.cap >= 10);

    arrClear(&arr);
}

void test_arrResizeToSameLen(void) {
    Arr(int) arr = { 0 };

    arrAppend(&arr, 1);
    arrAppend(&arr, 2);
    arrAppend(&arr, 3);

    size_t prevCap = arr.cap;
    int *items = arr.items;

    arrResize(&arr, arr.len);

    testAssert(arr.cap == prevCap);
    testAssert(arr.items == items);
    testAssert(arr.items[0] == 1);
    testAssert(arr.items[1] == 2);
    testAssert(arr.items[2] == 3);

    arrClear(&arr);
}

void test_arrResizeToBigger(void) {
    Arr(int) arr = { 0 };

    arrAppend(&arr, 1);
    arrAppend(&arr, 2);
    arrAppend(&arr, 3);

    arrResize(&arr, 10);

    testAssert(arr.cap >= 10);
    testAssert(arr.items[0] == 1);
    testAssert(arr.items[1] == 2);
    testAssert(arr.items[2] == 3);

    arrClear(&arr);
}

void test_arrResizeToSmaller(void) {
    Arr(int) arr = { 0 };

    arrAppend(&arr, 1);
    arrAppend(&arr, 2);
    arrAppend(&arr, 3);

    arrResize(&arr, 2);

    testAssert(arr.cap >= 2);
    testAssert(arr.len == 2);
    testAssert(arr.items[0] == 1);
    testAssert(arr.items[1] == 2);

    arrClear(&arr);
}

testList(
    testMake(test_arrResizeFromEmpty),
    testMake(test_arrResizeToSameLen),
    testMake(test_arrResizeToBigger),
    testMake(test_arrResizeToSmaller),
)
