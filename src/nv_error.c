#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif // !_WIN32

#include "nv_error.h"

static Err g_err = { 0 };

Err *errGet(void) {
    return &g_err;
}

static void printErrMsg_(const char *msg, char *desc) {
    if (msg == NULL || *msg == '\0') {
        (void)fprintf(stderr, "%s\r\n", desc);
    } else {
        (void)fprintf(stderr, "%s: %s\r\n", msg, desc);
    }
}

#ifdef _WIN32
#define msgBufSize 512

static void printWindowsError_(const char *msg) {
    static char msgBuf[msgBufSize];
    static TCHAR wMsgBuf[msgBufSize];
    DWORD formatFlags = FORMAT_MESSAGE_FROM_SYSTEM
                      | FORMAT_MESSAGE_IGNORE_INSERTS;
    DWORD errId = GetLastError();

    BOOL fmtResult = FormatMessageW(
        formatFlags,
        NULL,
        errId,
        0,
        wMsgBuf, msgBufSize,
        NULL
    );

    if (fmtResult == FALSE) {
        snprintf(
            msgBuf,
            msgBufSize,
            "failed to format message, error 0x%04lX", errId
        );
        printErrMsg_(msg, msgBuf);
    } else {
        ucdCh16StrToCh8Str(
            wMsgBuf, wcslen(wMsgBuf),
            (UcdCh8 *)msgBuf, msgBufSize
        );
        printErrMsg_(msg, msgBuf);
    }
}

#endif // !_WIN32

void errLog(const char *msg) {
    switch (g_err.type) {
    case ErrType_None:
        printErrMsg_(msg, (char *)"no error occurred");
        break;
    case ErrType_Errno:
#ifdef _WIN32
        printWindowsError_(msg);
#else
        printErrMsg_(msg, strerror(errno));
#endif // !_WIN32
        break;
    default:
        assert(false);
    }
}

void errSetErrno(void) {
    g_err.type = ErrType_Errno;
}

void errSetMsg(const char *msg) {
    g_err.type = ErrType_CustomMsg;
    g_err.data.customMsg = msg;
}
