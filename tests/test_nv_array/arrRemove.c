#include "nv_test.h"
#include "nv_array.h"

void test_arrRemoveBeginning(void) {
    Arr(int) arr = { 0 };
    arrAppend(&arr, 1);
    arrAppend(&arr, 2);
    arrAppend(&arr, 3);
    arrAppend(&arr, 4);

    arrRemove(&arr, 0);

    testAssert(arr.len == 3);
    testAssert(arr.items[0] == 2);
    testAssert(arr.items[1] == 3);
    testAssert(arr.items[2] == 4);

    arrClear(&arr);
}

void test_arrRemoveMiddle1(void) {
    Arr(int) arr = { 0 };
    arrAppend(&arr, 1);
    arrAppend(&arr, 2);
    arrAppend(&arr, 3);
    arrAppend(&arr, 4);

    arrRemove(&arr, 1);

    testAssert(arr.len == 3);
    testAssert(arr.items[0] == 1);
    testAssert(arr.items[1] == 3);
    testAssert(arr.items[2] == 4);

    arrClear(&arr);
}

void test_arrRemoveMiddle2(void) {
    Arr(int) arr = { 0 };
    arrAppend(&arr, 1);
    arrAppend(&arr, 2);
    arrAppend(&arr, 3);
    arrAppend(&arr, 4);

    arrRemove(&arr, 2);

    testAssert(arr.len == 3);
    testAssert(arr.items[0] == 1);
    testAssert(arr.items[1] == 2);
    testAssert(arr.items[2] == 4);

    arrClear(&arr);
}

void test_arrRemoveEnd(void) {
    Arr(int) arr = { 0 };
    arrAppend(&arr, 1);
    arrAppend(&arr, 2);
    arrAppend(&arr, 3);
    arrAppend(&arr, 4);

    arrRemove(&arr, 3);

    testAssert(arr.len == 3);
    testAssert(arr.items[0] == 1);
    testAssert(arr.items[1] == 2);
    testAssert(arr.items[2] == 3);

    arrClear(&arr);
}

testList(
    testMake(test_arrRemoveBeginning),
    testMake(test_arrRemoveMiddle1),
    testMake(test_arrRemoveMiddle2),
    testMake(test_arrRemoveEnd),
)
