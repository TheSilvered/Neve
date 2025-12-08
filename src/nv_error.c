#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include "nv_string.h"
#endif // !_WIN32

#include "nv_error.h"
#include "nv_threads.h"

static ThreadLocal Err g_err = { 0 };

Err *errGet(void) {
    return &g_err;
}

static void _printErrMsg(const char *msg, char *desc) {
    if (msg == NULL || *msg == '\0') {
        (void)fprintf(stderr, "%s\r\n", desc);
    } else {
        (void)fprintf(stderr, "%s: %s\r\n", msg, desc);
    }
}

#ifdef _WIN32

static void _printWindowsError(const char *msg) {
    static char msgBuf[512];
    static wchar_t wMsgBuf[512];
    DWORD formatFlags = FORMAT_MESSAGE_FROM_SYSTEM
                      | FORMAT_MESSAGE_IGNORE_INSERTS;
    DWORD errId = GetLastError();

    BOOL fmtResult = FormatMessageW(
        formatFlags,
        NULL,
        errId,
        0,
        wMsgBuf, nvArrlen(wMsgBuf),
        NULL
    );

    if (fmtResult == FALSE) {
        snprintf(
            msgBuf,
            nvArrlen(msgBuf),
            "failed to format message, error 0x%04lX", errId
        );
        _printErrMsg(msg, msgBuf);
    } else {
        ucdCh16StrToCh8Str(
            wMsgBuf, wcslen(wMsgBuf),
            (UcdCh8 *)msgBuf, nvArrlen(msgBuf)
        );
        _printErrMsg(msg, msgBuf);
    }
}

#endif // !_WIN32

void errLog(const char *msg) {
    switch (g_err.type) {
    case ErrType_None:
        _printErrMsg(msg, (char *)"no error occurred");
        break;
    case ErrType_Errno:
#ifdef _WIN32
        _printWindowsError(msg);
#else
        _printErrMsg(msg, strerror(errno));
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
