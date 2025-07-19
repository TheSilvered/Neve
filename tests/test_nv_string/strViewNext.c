#include "nv_test.h"
#include "nv_string.h"

void test_strViewNextEmpty(void) {
    StrView sv = {
        .buf = "",
        .len = 0
    };
    UcdCP cp;
    testAssert(strViewNext(&sv, -1, &cp) == -1);
    testAssert(cp == -1);
}

void test_strViewNextAscii(void) {
    StrView sv = {
        .buf = "abc",
        .len = 3
    };
    UcdCP cp;
    testAssert(strViewNext(&sv, -1, &cp) == 0);
    testAssert(cp == 'a');
    testAssert(strViewNext(&sv, 0, &cp) == 1);
    testAssert(cp == 'b');
    testAssert(strViewNext(&sv, 1, &cp) == 2);
    testAssert(cp == 'c');
    testAssert(strViewNext(&sv, 2, &cp) == -1);
    testAssert(cp == -1);
}

void test_strViewNextUTF8(void) {
    StrView sv = {
        .buf = "\x00\xc2\x80\xe0\xa0\x80\xf0\x90\x80\x80",
        .len = 10
    };
    UcdCP cp;
    testAssert(strViewNext(&sv, -1, &cp) == 0);
    testAssert(cp == 0x00);
    testAssert(strViewNext(&sv, 0, &cp) == 1);
    testAssert(cp == 0x80);
    testAssert(strViewNext(&sv, 1, &cp) == 3);
    testAssert(cp == 0x800);
    testAssert(strViewNext(&sv, 3, &cp) == 6);
    testAssert(cp == 0x10000);
    testAssert(strViewNext(&sv, 6, &cp) == -1);
    testAssert(cp == -1);
}

testList(
    testMake(test_strViewNextEmpty),
    testMake(test_strViewNextAscii),
    testMake(test_strViewNextUTF8)
)
