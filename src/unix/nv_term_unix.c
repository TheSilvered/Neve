#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "nv_error.h"
#include "nv_escapes.h"
#include "nv_term.h"
#include "nv_unicode.h"
#include "nv_utils.h"

static struct termios g_origTermios = { 0 };
static bool g_initialized = false;

// Initialization

bool termInit(void) {
    if (tcgetattr(STDIN_FILENO, &g_origTermios) != 0) {
        errSetErrno();
        return false;
    }
    g_initialized = true;
    return true;
}

bool termEnableRawMode(uint8_t getInputTimeoutDSec) {
    struct termios raw = g_origTermios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ISIG | ICANON | IEXTEN);

    if (getInputTimeoutDSec != 0) {
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = getInputTimeoutDSec;
    }

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) != 0) {
        errSetErrno();
        return false;
    }
    return true;
}

void termQuit(void) {
    if (g_initialized) {
        // Ignore failures, there is nothing left to do
        (void)tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_origTermios);
        g_initialized = false;
    }
}

// Input

UcdCP termGetInput(void) {
    UcdCh8 ch = 0;

    if (read(STDIN_FILENO, &ch, 1) < 0) {
        errSetErrno();
        return -1;
    } else if (ch == 0) {
        return 0;
    }

    // Read the full UTF-8 character
    UcdCh8 chBytes[4] = { ch, 0, 0, 0 };
    size_t chLen = ucdCh8RunLen(ch);

    // Do one less iteration as we already have the first byte
    for (size_t i = 1; i < chLen; i++) {
        ch = 0; // reset ch value for possible timeout of getCh
        if (read(STDIN_FILENO, &ch, 1) < 0) {
            errSetErrno();
            return -1;
        }
        if (ch == 0) {
            return chBytes[0];
        }
        chBytes[i] = ch;
    }

    return ucdCh8ToCP(chBytes);
}

int64_t termRead(UcdCh8 *buf, size_t bufSize) {
    ssize_t res = read(STDIN_FILENO, buf, bufSize);
    if (res < 0) {
        errSetErrno();
        return -1;
    }
    return res;
}

// Output

bool termWrite(const void *buf, size_t size) {
    if (write(STDOUT_FILENO, buf, size) == -1) {
        errSetErrno();
        return false;
    }
    return true;
}

bool termSize(uint16_t *outRows, uint16_t *outCols) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        errSetErrno();
        return false;
    }
    if (outRows != NULL) {
        *outRows = ws.ws_row;
    }
    if (outCols != NULL) {
        *outCols = ws.ws_col;
    }
    return true;
}

bool termCursorGetPos(uint16_t *outX, uint16_t *outY) {
    char buf[12]; // 12 = len(65535) + ';' + len(65535) + '\0'
    if (!termWrite(escCursorGetPos, 4)) {
        goto failure;
    }

    // Read as few characters as possible

    UcdCP ch = termGetInput();
    if (ch < 0) {
        goto failure;
    } else if (ch != '\x1b') {
        goto failure_msg;
    }

    ch = termGetInput();
    if (ch < 0) {
        goto failure;
    } else if (ch != '[') {
        goto failure_msg;
    }

    // In buf there will be <y>;<x>
    for (size_t i = 0; i < NV_ARRLEN(buf); i++) {
        // Going over the buffer size should not be possible considering a max
        // position of 16 bits (like the struct winsize fields).
        if (i == NV_ARRLEN(buf) - 1) {
            goto failure_msg;
        }

        ch = termGetInput();
        if (ch < 0) {
            goto failure;
        } else if (ch == 0 || ch > 0x7f) {
            goto failure_msg;
        } else if (ch == 'R') {
            buf[i + 1] = '\0';
            break;
        }
        buf[i] = (char)ch;
    }

    // Always fully parse both the x and the y even if not requested to check
    // for correctness.
    char *xEnd = NULL;
    long y = strtol(buf, &xEnd, 10);
    if (*xEnd != ';' || y <= 0) {
        goto failure_msg;
    }
    long x = strtol(xEnd + 1, NULL, 10);
    if (x <= 0) {
        goto failure_msg;
    }
    assert(x <= escNumMax);
    assert(y <= escNumMax);

    if (outX != NULL) {
        *outX = (uint16_t)(x - 1);
    }
    if (outY != NULL) {
        *outY = (uint16_t)(y - 1);
    }

    return true;

failure_msg:
    errSetMsg("invalid sequence when reading cursor position");

failure:
    if (outX != NULL) {
        *outX = 0;
    }
    if (outY != NULL) {
        *outY = 0;
    }
    return false;
}

bool termCursorSetPos(uint16_t x, uint16_t y) {
    char buf[15]; // 15 = ESC '[' len(65535) + ';' + len(65535) + '\0'
    x++;
    y++;

    if (x < UINT16_MAX) {
        x++;
    }
    if (y < UINT16_MAX) {
        y++;
    }

    size_t bufLen = snprintf(
        buf, NV_ARRLEN(buf),
        escCursorSetPos("%u", "%u"), y, x
    );

    if (bufLen == 0) {
        errSetErrno();
        return false;
    }

    if (!termWrite(buf, bufLen)) {
        return false;
    }

    return true;
}
