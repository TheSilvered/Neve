#include <string.h>

#include "nv_test.h"
#include "nv_str.h"

#define BUF_SIZE 10

void test_strBufAppendCEmpty(void) {
    char buf[BUF_SIZE];
    StrBuf sb = strBufMake(buf, BUF_SIZE);

    testAssert(strBufAppendC(&sb, ""));
    testAssert(sb.len == 0);
    testAssert(strcmp(sb.buf, "") == 0);
}

void test_strBufAppendCEmptyToExisting(void) {
    char buf[BUF_SIZE] = "a";
    StrBuf sb = {
        .buf = buf,
        .len = 1,
        .bufSize = BUF_SIZE
    };
    testAssert(strBufAppendC(&sb, ""));
    testAssert(sb.len == 1);
    testAssert(strcmp(sb.buf, "a") == 0);
}

void test_strBufAppendCFull(void) {
    char buf[BUF_SIZE];
    StrBuf sb = strBufMake(buf, BUF_SIZE);

    testAssert(strBufAppendC(&sb, "a"));
    testAssert(sb.len == 1);
    testAssert(strcmp(sb.buf, "a") == 0);
}

void test_strBufAppendCFullToExisting(void) {
    char buf[BUF_SIZE] = "a";
    StrBuf sb = {
        .buf = buf,
        .len = 1,
        .bufSize = BUF_SIZE
    };

    testAssert(strBufAppendC(&sb, "b"));
    testAssert(sb.len == 2);
    testAssert(strcmp(sb.buf, "ab") == 0);
}

#undef BUF_SIZE

void test_strBufAppendCTooBig(void) {
#define BUF_SIZE 1
    char buf[BUF_SIZE];
    StrBuf sb = strBufMake(buf, BUF_SIZE);

    testAssert(!strBufAppendC(&sb, "ab"));
    testAssert(sb.len == 0);
    testAssert(strcmp(sb.buf, "") == 0);
#undef BUF_SIZE
}

testList(
    testMake(test_strBufAppendCEmpty),
    testMake(test_strBufAppendCEmptyToExisting),
    testMake(test_strBufAppendCFull),
    testMake(test_strBufAppendCFullToExisting),
    testMake(test_strBufAppendCTooBig)
)
