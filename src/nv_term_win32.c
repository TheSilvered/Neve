#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE
#include <windows.h>

#include "nv_term.h"
#include "nv_unicode.h"

#define UNREACHABLE do { __assume(0); abort(); } while (0)

static HANDLE g_consoleInput = INVALID_HANDLE_VALUE;
static HANDLE g_consoleOutput = INVALID_HANDLE_VALUE;
static DWORD g_origInputMode = 0;
static DWORD g_origOutputMode = 0;
static TermErr g_error = { 0 };

static DWORD g_readTimeoutMs = 0;

#define W_MSG_BUF_SIZE 4096
#define MSG_BUF_SIZE 4096

static TCHAR g_wMsgBuf[MSG_BUF_SIZE];
static char g_msgBuf[MSG_BUF_SIZE];

#define INPUT_EVENTS_SIZE 512

static INPUT_RECORD g_inputEvents[INPUT_EVENTS_SIZE];
static size_t g_eventIdx = 0;
static size_t g_eventsSize = 0;

// Initialization

bool termInit(void) {
    g_consoleInput = GetStdHandle(STD_INPUT_HANDLE);
    if (g_consoleInput == INVALID_HANDLE_VALUE) {
        g_error.type = TermErrType_errno;
        return false;
    }

    if (GetConsoleMode(g_consoleInput, &g_origInputMode) == FALSE) {
        g_error.type = TermErrType_errno;
        return false;
    }

    g_consoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    if (g_consoleOutput == INVALID_HANDLE_VALUE) {
        g_error.type = TermErrType_errno;
        return false;
    }

    if (GetConsoleMode(g_consoleOutput, &g_origOutputMode) == FALSE) {
        g_error.type = TermErrType_errno;
        return false;
    }

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

bool termEnableRawMode(uint8_t getKeyTimeoutDSec) {
    DWORD inputMode = g_origInputMode;
    DWORD outputMode = g_origOutputMode;
    g_readTimeoutMs = getKeyTimeoutDSec * 100;

    inputMode &= ~(ENABLE_ECHO_INPUT
                 | ENABLE_LINE_INPUT
                 | ENABLE_PROCESSED_INPUT
                 | ENABLE_QUICK_EDIT_MODE
                 | ENABLE_MOUSE_INPUT
                 | ENABLE_WINDOW_INPUT);
    inputMode |= ENABLE_VIRTUAL_TERMINAL_INPUT;

    outputMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING
               | DISABLE_NEWLINE_AUTO_RETURN;

    if (SetConsoleMode(g_consoleInput, inputMode) == FALSE) {
        g_error.type = TermErrType_errno;
        return false;
    }

    if (SetConsoleMode(g_consoleOutput, outputMode) == FALSE) {
        g_error.type = TermErrType_errno;
        return false;
    }

    if (FlushConsoleInputBuffer(g_consoleInput) == FALSE) {
        g_error.type = TermErrType_errno;
        return false;
    }

    return true;
}

void termQuit(void) {
    // Ignore failures, there is nothing left to do
    (void)SetConsoleMode(g_consoleInput,  g_origInputMode);
    (void)SetConsoleMode(g_consoleOutput,  g_origOutputMode);
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
            g_wMsgBuf, W_MSG_BUF_SIZE,
            NULL
        );

        if (fmtResult == FALSE) {
            snprintf(
                g_msgBuf,
                MSG_BUF_SIZE,
                "failed to format message, error 0x%04lX", errId
            );
            printErrMsg(msg, g_msgBuf);
        } else {
            ucdUTF16ToUTF8(
                g_wMsgBuf, wcslen(g_wMsgBuf),
                (UcdCh8 *)g_msgBuf, MSG_BUF_SIZE
            );
            printErrMsg(msg, g_msgBuf);
        }
        break;
    }
    default:
        UNREACHABLE;

    }
}

// Input

static int getCh(UcdCh8 *outCh) {
    if (g_readTimeoutMs == 0) {
        DWORD charsRead;
        if (ReadConsoleA(g_consoleInput, outCh, 1, &charsRead, NULL) == FALSE) {
            g_error.type = TermErrType_errno;
            return -1;
        }
        return (int)charsRead;
    }

    while (g_eventIdx < g_eventsSize) {
        INPUT_RECORD *event = &g_inputEvents[g_eventIdx++];
        switch (event->EventType) {
        case MOUSE_EVENT:
        case WINDOW_BUFFER_SIZE_EVENT:
        case FOCUS_EVENT:
        case MENU_EVENT:
            continue;
        case KEY_EVENT:
            if (event->Event.KeyEvent.bKeyDown) {
                *outCh = (UcdCh8)event->Event.KeyEvent.uChar.UnicodeChar;
                return 1;
            } else {
                continue;
            }
        }
    }

    g_eventIdx = 0;
    g_eventsSize = 0;

    switch (WaitForSingleObject(g_consoleInput, 100)) {
    case WAIT_ABANDONED:
        return 0;
    case WAIT_TIMEOUT:
        return 0;
    case WAIT_FAILED:
        g_error.type = TermErrType_errno;
        return -1;
    case WAIT_OBJECT_0:
        break;
    default:
        UNREACHABLE;
    }

    DWORD eventsRead;

    BOOL result = ReadConsoleInputA(
        g_consoleInput,
        g_inputEvents, INPUT_EVENTS_SIZE,
        &eventsRead
    );
    g_eventsSize = eventsRead;
    if (result == FALSE) {
        g_error.type = TermErrType_errno;
        return -1;
    } else if (g_eventsSize == 0) {
        // this should be unreachable but in any case avoids infinite recursion
        return 0;
    } else {
        return getCh(outCh);
    }
}

TermKey termGetKey(void) {
    // TODO: change input CP to UTF-16 because emojis don't work
    UcdCh8 ch = 0;

    if (getCh(&ch) < 0) {
        g_error.type = TermErrType_errno;
        return -1;
    } else if (ch == 0) {
        return 0;
    }

    // Read the full UTF-8 character
    UcdCh8 chBytes[4] = { ch, 0, 0, 0 };
    size_t chLen = ucdUTF8ByteLen(ch);

    // Do one less iteration as we already have the first byte
    for (size_t i = 1; i < chLen; i++) {
        ch = 0; // reset ch value for possible timeout of getCh
        if (getCh(&ch) < 0) {
            return -1;
        } else if (ch == 0) {
            return (TermKey)chBytes[0];
        }
        chBytes[i] = ch;
    }

    return ucdCh8ToCh32(chBytes);
}

// Output

bool termWrite(const void *buf, size_t size) {
    if (WriteConsoleA(g_consoleOutput, buf, (DWORD)size, NULL, NULL) == FALSE) {
        g_error.type = TermErrType_errno;
        return false;
    }
    return true;
}
