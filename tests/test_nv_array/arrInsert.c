#include "nv_test.h"
#include "nv_array.h"

void test_arrInsertBeginning(void) {
    Arr(int) arr = { 0 };
    arrAppend(&arr, 1);
    arrAppend(&arr, 2);
    arrAppend(&arr, 3);
    arrAppend(&arr, 4);

    arrInsert(&arr, 0, 10);

    testAssert(arr.len == 5);
    testAssert(arr.items[0] == 10);
    testAssert(arr.items[1] == 1);
    testAssert(arr.items[2] == 2);
    testAssert(arr.items[3] == 3);
    testAssert(arr.items[4] == 4);

    arrDestroy(&arr);
}

void test_arrInsertMiddle1(void) {
    Arr(int) arr = { 0 };
    arrAppend(&arr, 1);
    arrAppend(&arr, 2);
    arrAppend(&arr, 3);
    arrAppend(&arr, 4);

    arrInsert(&arr, 1, 10);

    testAssert(arr.len == 5);
    testAssert(arr.items[0] == 1);
    testAssert(arr.items[1] == 10);
    testAssert(arr.items[2] == 2);
    testAssert(arr.items[3] == 3);
    testAssert(arr.items[4] == 4);

    arrDestroy(&arr);
}

void test_arrInsertMiddle2(void) {
    Arr(int) arr = { 0 };
    arrAppend(&arr, 1);
    arrAppend(&arr, 2);
    arrAppend(&arr, 3);
    arrAppend(&arr, 4);

    arrInsert(&arr, 2, 10);

    testAssert(arr.len == 5);
    testAssert(arr.items[0] == 1);
    testAssert(arr.items[1] == 2);
    testAssert(arr.items[2] == 10);
    testAssert(arr.items[3] == 3);
    testAssert(arr.items[4] == 4);

    arrDestroy(&arr);
}

void test_arrInsertEnd(void) {
    Arr(int) arr = { 0 };
    arrAppend(&arr, 1);
    arrAppend(&arr, 2);
    arrAppend(&arr, 3);
    arrAppend(&arr, 4);

    arrInsert(&arr, 3, 10);

    testAssert(arr.len == 5);
    testAssert(arr.items[0] == 1);
    testAssert(arr.items[1] == 2);
    testAssert(arr.items[2] == 3);
    testAssert(arr.items[3] == 10);
    testAssert(arr.items[4] == 4);

    arrDestroy(&arr);
}

testList(
    testMake(test_arrInsertBeginning),
    testMake(test_arrInsertMiddle1),
    testMake(test_arrInsertMiddle2),
    testMake(test_arrInsertEnd),
)
