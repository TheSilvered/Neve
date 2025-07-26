#include <string.h>

#include "nv_test.h"
#include "nv_string.h"

void test_strReserveZero(void) {
    Str str;

    strInit(&str, 0);
    strReserve(&str, 0);

    strDestroy(&str);
}

void test_strReserveNonZero(void) {
    size_t reserve = 10;
    Str str;

    strInit(&str, 0);
    strReserve(&str, reserve);
    testAssert(str.cap >= reserve);

    strDestroy(&str);
}

void test_strReserveFromFull(void) {
    const char *cStr = "a";
    size_t cStrLen = strlen(cStr);
    size_t reserve = 10;
    Str str;

    strInitFromC(&str, cStr);
    strReserve(&str, reserve);
    testAssert(str.cap >= reserve + cStrLen);
    testAssert(strcmp(strAsC(&str), cStr) == 0);

    strDestroy(&str);
}

void test_strReservePreventsRealloc(void) {
    const char *cStr = "abc";
    const char *longCStr = "defghijklmnopqrstuvwxyz";
    const char *result = "abcdefghijklmnopqrstuvwxyz";
    size_t cStrLen = strlen(cStr);
    size_t reserve = strlen(longCStr);
    Str str;

    strInitFromC(&str, cStr);
    strReserve(&str, reserve);
    testAssert(str.cap >= reserve + cStrLen);

    UcdCh8 *prevBuf = str.buf;

    strAppendC(&str, longCStr);
    testAssert(strcmp(strAsC(&str), result) == 0);
    testAssert(str.buf == prevBuf);

    strDestroy(&str);
}

testList(
    testMake(test_strReserveZero),
    testMake(test_strReserveNonZero),
    testMake(test_strReserveFromFull),
    testMake(test_strReservePreventsRealloc)
)
