#include <string.h>

#include "nv_test.h"
#include "nv_string.h"

void test_strBufAppendCEmpty(void) {
    char buf[10];
    StrBuf sb = strBufMake(buf, nvArrlen(buf));

    testAssert(strBufAppendC(&sb, ""));
    testAssert(sb.len == 0);
    testAssert(strcmp(sb.buf, "") == 0);
}

void test_strBufAppendCEmptyToExisting(void) {
    char buf[10] = "a";
    StrBuf sb = {
        .buf = buf,
        .len = 1,
        .bufSize = nvArrlen(buf)
    };
    testAssert(strBufAppendC(&sb, ""));
    testAssert(sb.len == 1);
    testAssert(strcmp(sb.buf, "a") == 0);
}

void test_strBufAppendCFull(void) {
    char buf[10];
    StrBuf sb = strBufMake(buf, nvArrlen(buf));

    testAssert(strBufAppendC(&sb, "a"));
    testAssert(sb.len == 1);
    testAssert(strcmp(sb.buf, "a") == 0);
}

void test_strBufAppendCFullToExisting(void) {
    char buf[10] = "a";
    StrBuf sb = {
        .buf = buf,
        .len = 1,
        .bufSize = nvArrlen(buf)
    };

    testAssert(strBufAppendC(&sb, "b"));
    testAssert(sb.len == 2);
    testAssert(strcmp(sb.buf, "ab") == 0);
}

void test_strBufAppendCTooBig(void) {
    char buf[1];
    StrBuf sb = strBufMake(buf, nvArrlen(buf));

    testAssert(!strBufAppendC(&sb, "ab"));
    testAssert(sb.len == 0);
    testAssert(strcmp(sb.buf, "") == 0);
}

testList(
    testMake(test_strBufAppendCEmpty),
    testMake(test_strBufAppendCEmptyToExisting),
    testMake(test_strBufAppendCFull),
    testMake(test_strBufAppendCFullToExisting),
    testMake(test_strBufAppendCTooBig)
)
