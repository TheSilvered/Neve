#include "sterm.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE
#include <windows.h>

#define UNREACHABLE __assume(0)

static HANDLE g_consoleInput = INVALID_HANDLE_VALUE;
static DWORD g_origInputMode = 0;
static HANDLE g_consoleOutput = INVALID_HANDLE_VALUE;
static DWORD g_origOutputMode = 0;
static HANDLE g_consoleError = INVALID_HANDLE_VALUE;
static DWORD g_origErrorMode = 0;
static TermErr g_error = { 0 };

#define W_MSG_BUF_SIZE 4096
#define MSG_BUF_SIZE 4096

static TCHAR wMsgBuf[MSG_BUF_SIZE];
static char msgBuf[MSG_BUF_SIZE];

typedef int32_t unicode_t;

// Windows utilities

static size_t wcharToChar(
    wchar_t *str, size_t strLen,
    char *buf, size_t bufSize
) {
    size_t bufIdx = 0;
    for (size_t strIdx = 0; strIdx < strLen;) {
        wchar_t wch = str[strIdx++];
        unicode_t unicodeCP = 0;
        if (wch < 0xd800 || wch > 0xdfff) {
            unicodeCP = wch;
        } else {
            if (strIdx == strLen) {
                break;
            }
            unicodeCP = ((wch & 0x3ff) << 10)
                      + (str[strIdx++] & 0x3ff)
                      + 0x10000;
        }

        if (unicodeCP <= 0x7f && bufSize - bufIdx > 1) {
            buf[bufIdx++] = (char)unicodeCP;
        } else if (unicodeCP <= 0x7ff && bufSize - bufIdx > 2) {
            buf[bufIdx++] = 0b11000000 | (char)(unicodeCP >> 6);
            buf[bufIdx++] = 0b10000000 | (char)(unicodeCP & 0x3f);
        } else if (unicodeCP <= 0xffff && bufSize - bufIdx > 3) {
            buf[bufIdx++] = 0b11100000 | (char)(unicodeCP >> 12);
            buf[bufIdx++] = 0b10000000 | (char)(unicodeCP >> 6 & 0x3f);
            buf[bufIdx++] = 0b10000000 | (char)(unicodeCP & 0x3f);
        } else if (unicodeCP <= 0x10fffe && bufSize - bufIdx > 4) {
            buf[bufIdx++] = 0b11110000 | (char)(unicodeCP >> 18);
            buf[bufIdx++] = 0b10000000 | (char)(unicodeCP >> 12 & 0x3f);
            buf[bufIdx++] = 0b10000000 | (char)(unicodeCP >> 6 & 0x3f);
            buf[bufIdx++] = 0b10000000 | (char)(unicodeCP & 0x3f);
        } else
            break;
    }
    buf[bufIdx] = '\0';
    return bufIdx;
}

// Initialization

static bool initHandle(DWORD handleNo, HANDLE *outHandle, DWORD *outMode) {
    *outHandle = GetStdHandle(handleNo);
    if (*outHandle == INVALID_HANDLE_VALUE) {
        g_error.type = TermErrType_errno;
        return false;
    }

    if (GetConsoleMode(g_consoleInput, outMode) == FALSE) {
        g_error.type = TermErrType_errno;
        return false;
    }

    return true;
}

bool termInit(void) {
    if (!initHandle(STD_INPUT_HANDLE, &g_consoleInput, &g_origInputMode))
        return false;
    if (!initHandle(STD_OUTPUT_HANDLE, &g_consoleOutput, &g_origOutputMode))
        return false;
    if (!initHandle(STD_ERROR_HANDLE,  &g_consoleError,  &g_origErrorMode))
        return false;
    if (!SetConsoleCP(CP_UTF8)) {
        g_error.type = TermErrType_errno;
        return false;
    }
    if (!SetConsoleOutputCP(CP_UTF8)) {
        g_error.type = TermErrType_errno;
        return false;
    }
    return true;
}

bool termEnableRawMode(void) {
    DWORD inputMode  = g_origInputMode;

    inputMode &= ~(ENABLE_ECHO_INPUT
                 | ENABLE_LINE_INPUT
                 | ENABLE_PROCESSED_INPUT
                 | ENABLE_QUICK_EDIT_MODE);
    inputMode |= ENABLE_VIRTUAL_TERMINAL_INPUT;

    if (SetConsoleMode(g_consoleInput, inputMode) == FALSE) {
        g_error.type = TermErrType_errno;
        return false;
    }

    return true;
}

void termQuit(void) {
    // Ignore failures, there is nothing left to do
    (void)SetConsoleMode(g_consoleInput,  g_origInputMode);
    (void)SetConsoleMode(g_consoleOutput, g_origInputMode);
    (void)SetConsoleMode(g_consoleError,  g_origInputMode);
}

// Error handling

TermErr *termErr(void) {
    return &g_error;
}

static void printErrMsg(const char *msg, char *desc) {
    if (msg == NULL || *msg == '\0') {
        (void)fprintf(stderr, "%s\r\n", desc);
    } else {
        (void)fprintf(stderr, "%s: %.4096s\r\n", msg, desc);
    }
}

void termLogError(const char *msg) {
    switch (g_error.type) {
    case TermErrType_none:
        printErrMsg(msg, (char *)"no error occurred");
        break;
    case TermErrType_errno: {
        DWORD formatFlags = FORMAT_MESSAGE_FROM_SYSTEM
                          | FORMAT_MESSAGE_IGNORE_INSERTS;
        DWORD errId = GetLastError();

        BOOL fmtResult = FormatMessageW(
            formatFlags,
            NULL,
            errId,
            0,
            wMsgBuf, W_MSG_BUF_SIZE,
            NULL
        );

        if (fmtResult == FALSE) {
            snprintf(
                msgBuf,
                MSG_BUF_SIZE,
                "failed to format message, error 0x%04lX", errId);
            printErrMsg(msg, msgBuf);
        } else {
            wcharToChar(wMsgBuf, wcslen(wMsgBuf), msgBuf, MSG_BUF_SIZE);
            printErrMsg(msg, msgBuf);
        }
        break;
    }
    default:
        UNREACHABLE;
    }
}

// Input

TermKey termGetKey(void) {
    wchar_t ch;
    DWORD charsRead;
    if (ReadConsoleW(g_consoleInput, &ch, 1, &charsRead, NULL) == FALSE) {
        g_error.type = TermErrType_errno;
        return -1;
    }

    if (charsRead < 1) {
        return 0;
    } else {
        return (TermKey)ch;
    }
}
