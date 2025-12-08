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

#include "nv_error.h"
#include "nv_term.h"
#include "nv_unicode.h"
#include "nv_utils.h"

static HANDLE g_consoleInput = INVALID_HANDLE_VALUE;
static HANDLE g_consoleOutput = INVALID_HANDLE_VALUE;
static DWORD g_origInputMode = 0;
static DWORD g_origOutputMode = 0;

static DWORD g_readTimeoutMs = 0;

static INPUT_RECORD g_inputEvents[512];
static size_t g_eventIdx = 0;
static size_t g_eventsSize = 0;
static bool g_initialized = false;

// Initialization

bool termInit(void) {
    g_consoleInput = GetStdHandle(STD_INPUT_HANDLE);
    if (g_consoleInput == INVALID_HANDLE_VALUE) {
        errSetErrno();
        return false;
    }

    if (GetConsoleMode(g_consoleInput, &g_origInputMode) == FALSE) {
        errSetErrno();
        return false;
    }

    g_consoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    if (g_consoleOutput == INVALID_HANDLE_VALUE) {
        errSetErrno();
        return false;
    }

    if (GetConsoleMode(g_consoleOutput, &g_origOutputMode) == FALSE) {
        errSetErrno();
        return false;
    }

    if (!SetConsoleOutputCP(CP_UTF8)) {
        errSetErrno();
        return false;
    }
    g_initialized = true;
    return true;
}

bool termEnableRawMode(uint8_t getInputTimeoutDSec) {
    DWORD inputMode = g_origInputMode;
    DWORD outputMode = g_origOutputMode;
    g_readTimeoutMs = getInputTimeoutDSec * 100;

    inputMode &= ~(ENABLE_ECHO_INPUT
                 | ENABLE_LINE_INPUT
                 | ENABLE_PROCESSED_INPUT
                 | ENABLE_QUICK_EDIT_MODE
                 | ENABLE_MOUSE_INPUT
                 | ENABLE_WINDOW_INPUT);
    inputMode |= ENABLE_VIRTUAL_TERMINAL_INPUT;

    outputMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    if (SetConsoleMode(g_consoleInput, inputMode) == FALSE) {
        errSetErrno();
        return false;
    }

    if (SetConsoleMode(g_consoleOutput, outputMode) == FALSE) {
        errSetErrno();
        return false;
    }

    if (FlushConsoleInputBuffer(g_consoleInput) == FALSE) {
        errSetErrno();
        return false;
    }

    return true;
}

void termQuit(void) {
    if (g_initialized) {
        // Ignore failures, there is nothing left to do
        (void)SetConsoleMode(g_consoleInput,  g_origInputMode);
        (void)SetConsoleMode(g_consoleOutput,  g_origOutputMode);
        g_initialized = false;
    }
}

// Input

static int getCh(UcdCh16 *outCh) {
    if (g_readTimeoutMs == 0) {
        DWORD charsRead;
        if (ReadConsoleW(g_consoleInput, outCh, 1, &charsRead, NULL) == FALSE) {
            errSetErrno();
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
                *outCh = (UcdCh16)event->Event.KeyEvent.uChar.UnicodeChar;
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
        errSetErrno();
        return -1;
    case WAIT_OBJECT_0:
        break;
    default:
        nvUnreachable;
    }

    DWORD eventsRead;

    BOOL result = ReadConsoleInputW(
        g_consoleInput,
        g_inputEvents, nvArrlen(g_inputEvents),
        &eventsRead
    );
    g_eventsSize = eventsRead;
    if (result == FALSE) {
        errSetErrno();
        return -1;
    } else if (g_eventsSize == 0) {
        // this should be unreachable but in any case avoids infinite recursion
        return 0;
    } else {
        return getCh(outCh);
    }
}

UcdCP termGetInput(void) {
    UcdCh16 ch = 0;

    if (getCh(&ch) < 0) {
        errSetErrno();
        return -1;
    } else if (ch == 0) {
        return 0;
    }

    // Read the full UTF-16 character
    UcdCh16 chBytes[2] = { ch, 0 };
    uint8_t chLen = ucdCh16RunLen(ch);
    if (chLen == 1) {
        return ch;
    } else if (chLen == 2) {
        ch = 0;
        if (getCh(&ch) < 0) {
            return -1;
        } else if (ch == 0) {
            return (UcdCP)chBytes[0];
        }
        chBytes[1] = ch;
        return ucdCh16ToCP(chBytes);
    } else {
        return 0;
    }
}

int64_t termRead(UcdCh8 *buf, size_t bufSize) {
    wchar_t readBuf[4096] = { 0 };
    size_t toRead = nvMin(bufSize, nvArrlen(readBuf));
    size_t idx = 0;
    uint8_t offsetOutBuf = 0;

    while (toRead) {
        DWORD charsRead;
        BOOL readResult = ReadConsoleW(
            g_consoleInput,
            readBuf + offsetOutBuf,
            (DWORD)toRead,
            &charsRead,
            NULL
        );
        if (readResult == FALSE) {
            errSetErrno();
            return -1;
        }
        charsRead += offsetOutBuf;

        // If the read happened to stop in the middle of a code point
        if (charsRead > 1 && ucdCh16RunLen(readBuf[charsRead - 1]) == 2) {
            offsetOutBuf = 1;
            charsRead -= 1;
        } else {
            offsetOutBuf = 0;
        }

        idx += ucdCh16StrToCh8Str(
            readBuf,
            charsRead,
            buf + idx,
            bufSize - idx
        );

        if (offsetOutBuf) {
            readBuf[0] = readBuf[charsRead - 1];
        }

        if (charsRead < toRead) {
            toRead = 0;
        } else {
            toRead = nvMin(bufSize - idx, nvArrlen(readBuf) - offsetOutBuf);
        }
    }

    if (offsetOutBuf) {
        idx += ucdCh16StrToCh8Str(readBuf, 1, buf + idx, bufSize - idx);
    }

    return idx;
}

// Output

bool termWrite(const void *buf, size_t size) {
    if (WriteConsoleA(g_consoleOutput, buf, (DWORD)size, NULL, NULL) == FALSE) {
        errSetErrno();
        return false;
    }
    return true;
}

bool termSize(uint16_t *outRows, uint16_t *outCols) {
    CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
    if (GetConsoleScreenBufferInfo(g_consoleOutput, &bufferInfo) == FALSE) {
        errSetErrno();
        if (outRows != NULL) {
            *outRows = 0;
        }
        if (outCols != NULL) {
            *outCols = 0;
        }
        return false;
    }
    if (outRows != NULL) {
        *outRows = (uint16_t)bufferInfo.dwSize.Y;
    }
    if (outCols != NULL) {
        *outCols = (uint16_t)bufferInfo.dwSize.X;
    }
    return true;
}

bool termCursorGetPos(uint16_t *outX, uint16_t *outY) {
    CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
    if (GetConsoleScreenBufferInfo(g_consoleOutput, &bufferInfo) == FALSE) {
        errSetErrno();
        if (outX != NULL) {
            *outX = 0;
        }
        if (outY != NULL) {
            *outY = 0;
        }
        return false;
    }
    if (outX != NULL) {
        *outX = (uint16_t)bufferInfo.dwCursorPosition.X;
    }
    if (outY != NULL) {
        *outY = (uint16_t)bufferInfo.dwCursorPosition.Y;
    }
    return true;
}

bool termCursorSetPos(uint16_t x, uint16_t y) {
    if (x > INT16_MAX) {
        x = INT16_MAX;
    }
    if (y > INT16_MAX) {
        y = INT16_MAX;
    }
    COORD cursorPos = { .X = x, .Y = y };
    if (SetConsoleCursorPosition(g_consoleOutput, cursorPos) == FALSE) {
        errSetErrno();
        return false;
    }
    return true;
}
