#include <string.h>

#include "nv_test.h"
#include "nv_str.h"

#define BUF_SIZE 10

void test_strBufAppendEmpty(void) {
    char buf[BUF_SIZE];
    StrBuf sb = strBufMake(buf, BUF_SIZE);
    StrView emptyView = strViewMakeFromC("");

    testAssert(strBufAppend(&sb, &emptyView));
    testAssert(sb.len == 0);
    testAssert(strcmp(sb.buf, "") == 0);
}

void test_strBufAppendEmptyToExisting(void) {
    char buf[BUF_SIZE] = "a";
    StrBuf sb = {
        .buf = buf,
        .len = 1,
        .bufSize = BUF_SIZE
    };
    StrView emptyView = strViewMakeFromC("");

    testAssert(strBufAppend(&sb, &emptyView));
    testAssert(sb.len == 1);
    testAssert(strcmp(sb.buf, "a") == 0);
}

void test_strBufAppendFull(void) {
    char buf[BUF_SIZE];
    StrBuf sb = strBufMake(buf, BUF_SIZE);
    StrView fullView = strViewMakeFromC("a");

    testAssert(strBufAppend(&sb, &fullView));
    testAssert(sb.len == 1);
    testAssert(strcmp(sb.buf, "a") == 0);
}

void test_strBufAppendFullToExisting(void) {
    char buf[BUF_SIZE] = "a";
    StrBuf sb = {
        .buf = buf,
        .len = 1,
        .bufSize = BUF_SIZE
    };
    StrView fullView = strViewMakeFromC("b");

    testAssert(strBufAppend(&sb, &fullView));
    testAssert(sb.len == 2);
    testAssert(strcmp(sb.buf, "ab") == 0);
}

#undef BUF_SIZE

void test_strBufAppendTooBig(void) {
#define BUF_SIZE 1
    char buf[BUF_SIZE];
    StrBuf sb = strBufMake(buf, BUF_SIZE);
    StrView fullView = strViewMakeFromC("ab");

    testAssert(!strBufAppend(&sb, &fullView));
    testAssert(sb.len == 0);
    testAssert(strcmp(sb.buf, "") == 0);
#undef BUF_SIZE
}

testList(
    testMake(test_strBufAppendEmpty),
    testMake(test_strBufAppendEmptyToExisting),
    testMake(test_strBufAppendFull),
    testMake(test_strBufAppendFullToExisting),
    testMake(test_strBufAppendTooBig)
)
