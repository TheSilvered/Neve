#include "nv_test.h"
#include "nv_escapes.h"
#define _readBufSize 4
#include "nv_buffer.c"

void test_bufInitFromFileEmpty(void) {
    BufMap map;
    bufMapInit(&map);

    File temp;
    testAssertRequire(fileOpenTemp(&temp) == FileIOResult_Success);

    BufHandle hd;
    BufResult res = bufInitFromFile(&map, &temp, &hd);

    fileClose(&temp);

    testAssertRequire(hd != bufInvalidHandle);
    testAssert(res.kind == BufResult_Success);
    Buf *buf = bufRef(&map, hd);
    StrView content = ctxGetContent(&buf->ctx);
    testAssert(content.len == 0);

    bufClose(&map, hd);
    bufMapDestroy(&map);
}

void test_bufInitFromFileSmall(void) {
    BufMap map;
    bufMapInit(&map);

    File temp;
    testAssertRequire(fileOpenTemp(&temp) == FileIOResult_Success);
    testAssertRequire(fileWrite(&temp, sLen("abc")) == FileIOResult_Success);
    filePosToBeginning(&temp);

    BufHandle hd;
    BufResult res = bufInitFromFile(&map, &temp, &hd);

    fileClose(&temp);

    testAssertRequire(hd != bufInvalidHandle);
    testAssert(res.kind == BufResult_Success);
    Buf *buf = bufRef(&map, hd);
    StrView content = ctxGetContent(&buf->ctx);
    testAssert(content.len == 3);
    testAssert(strncmp((char *)content.buf, "abc", content.len) == 0);

    bufClose(&map, hd);
    bufMapDestroy(&map);
}

void test_bufInitFromFileValid(void) {
    BufMap map;
    bufMapInit(&map);

    File temp;
    testAssertRequire(fileOpenTemp(&temp) == FileIOResult_Success);
    testAssertRequire(
        fileWrite(&temp, sLen("ab\xf0\x9f\x98\x8a")) == FileIOResult_Success
    );
    filePosToBeginning(&temp);

    BufHandle hd;
    BufResult res = bufInitFromFile(&map, &temp, &hd);

    fileClose(&temp);

    testAssertRequire(hd != bufInvalidHandle);
    testAssert(res.kind == BufResult_Success);
    Buf *buf = bufRef(&map, hd);
    StrView content = ctxGetContent(&buf->ctx);
    testAssert(content.len == 6);
    testAssert(
        strncmp((char *)content.buf, "ab\xf0\x9f\x98\x8a", content.len) == 0
    );

    bufClose(&map, hd);
    bufMapDestroy(&map);
}

void test_bufInitFromFileValid2(void) {
    BufMap map;
    bufMapInit(&map);

    File temp;
    testAssertRequire(fileOpenTemp(&temp) == FileIOResult_Success);
    testAssertRequire(
        fileWrite(&temp, sLen("abcd\xf0\x9f\x98\x8a")) == FileIOResult_Success
    );
    filePosToBeginning(&temp);

    BufHandle hd;
    BufResult res = bufInitFromFile(&map, &temp, &hd);

    fileClose(&temp);

    testAssertRequire(hd != bufInvalidHandle);
    testAssert(res.kind == BufResult_Success);
    Buf *buf = bufRef(&map, hd);
    StrView content = ctxGetContent(&buf->ctx);
    testAssert(content.len == 8);
    testAssert(
        strncmp((char *)content.buf, "abcd\xf0\x9f\x98\x8a", content.len) == 0
    );

    bufClose(&map, hd);
    bufMapDestroy(&map);
}

void test_bufInitFromFileInvalid(void) {
    BufMap map;
    bufMapInit(&map);

    File temp;
    testAssertRequire(fileOpenTemp(&temp) == FileIOResult_Success);
    testAssertRequire(
        fileWrite(&temp, sLen("a\xff")) == FileIOResult_Success
    );
    filePosToBeginning(&temp);

    BufHandle hd;
    BufResult res = bufInitFromFile(&map, &temp, &hd);

    fileClose(&temp);

    testAssertRequire(hd == bufInvalidHandle);
    testAssert(res.kind == BufResult_EncodingError);

    bufMapDestroy(&map);
}

testList(
    testMake(test_bufInitFromFileEmpty),
    testMake(test_bufInitFromFileSmall),
    testMake(test_bufInitFromFileValid),
    testMake(test_bufInitFromFileValid2),
    testMake(test_bufInitFromFileInvalid)
)
