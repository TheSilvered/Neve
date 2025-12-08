#include <string.h>

#include "nv_test.h"
#include "nv_string.h"

void test_strBufAppendEmpty(void) {
    char buf[10];
    StrBuf sb = strBufMake(buf, nvArrlen(buf));
    StrView emptyView = strViewMakeFromC("");

    testAssert(strBufAppend(&sb, &emptyView));
    testAssert(sb.len == 0);
    testAssert(strcmp(sb.buf, "") == 0);
}

void test_strBufAppendEmptyToExisting(void) {
    char buf[10] = "a";
    StrBuf sb = {
        .buf = buf,
        .len = 1,
        .bufSize = nvArrlen(buf)
    };
    StrView emptyView = strViewMakeFromC("");

    testAssert(strBufAppend(&sb, &emptyView));
    testAssert(sb.len == 1);
    testAssert(strcmp(sb.buf, "a") == 0);
}

void test_strBufAppendFull(void) {
    char buf[10];
    StrBuf sb = strBufMake(buf, nvArrlen(buf));
    StrView fullView = strViewMakeFromC("a");

    testAssert(strBufAppend(&sb, &fullView));
    testAssert(sb.len == 1);
    testAssert(strcmp(sb.buf, "a") == 0);
}

void test_strBufAppendFullToExisting(void) {
    char buf[10] = "a";
    StrBuf sb = {
        .buf = buf,
        .len = 1,
        .bufSize = nvArrlen(buf)
    };
    StrView fullView = strViewMakeFromC("b");

    testAssert(strBufAppend(&sb, &fullView));
    testAssert(sb.len == 2);
    testAssert(strcmp(sb.buf, "ab") == 0);
}

void test_strBufAppendTooBig(void) {
    char buf[1];
    StrBuf sb = strBufMake(buf, nvArrlen(buf));
    StrView fullView = strViewMakeFromC("ab");

    testAssert(!strBufAppend(&sb, &fullView));
    testAssert(sb.len == 0);
    testAssert(strcmp(sb.buf, "") == 0);
}

testList(
    testMake(test_strBufAppendEmpty),
    testMake(test_strBufAppendEmptyToExisting),
    testMake(test_strBufAppendFull),
    testMake(test_strBufAppendFullToExisting),
    testMake(test_strBufAppendTooBig)
)
