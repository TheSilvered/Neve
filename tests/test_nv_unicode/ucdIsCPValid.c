#include "nv_test.h"
#include "nv_unicode.h"

void test_ucdIsCPValidU0000(void) {
    testAssert(ucdIsCPValid(0x0000));
}

void test_ucdIsCPValidU10FFFF(void) {
    testAssert(ucdIsCPValid(0x10FFFF));
}

void test_ucdIsCPValidTooBig(void) {
    testAssert(!ucdIsCPValid(0x110000));
}

void test_ucdIsCPValidNegative(void) {
    testAssert(!ucdIsCPValid(-1));
}

void test_ucdIsCPValidFirstHighSurrogate(void) {
    testAssert(!ucdIsCPValid(ucdHighSurrogateFirst));
}

void test_ucdIsCPValidLastHighSurrogate(void) {
    testAssert(!ucdIsCPValid(ucdHighSurrogateLast));
}

void test_ucdIsCPValidFirstLowSurrogate(void) {
    testAssert(!ucdIsCPValid(ucdLowSurrogateFirst));
}

void test_ucdIsCPValidLastLowSurrogate(void) {
    testAssert(!ucdIsCPValid(ucdLowSurrogateLast));
}

testList(
    testMake(test_ucdIsCPValidU0000),
    testMake(test_ucdIsCPValidU10FFFF),
    testMake(test_ucdIsCPValidTooBig),
    testMake(test_ucdIsCPValidNegative),
    testMake(test_ucdIsCPValidFirstHighSurrogate),
    testMake(test_ucdIsCPValidLastHighSurrogate),
    testMake(test_ucdIsCPValidFirstLowSurrogate),
    testMake(test_ucdIsCPValidLastLowSurrogate)
)
