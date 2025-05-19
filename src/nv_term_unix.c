#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>

#include "nv_term.h"
#include "nv_unicode.h"

#define UNREACHABLE __builtin_unreachable()
#define CURSOR_POS_BUF_SIZE 12 // len(65535) + ';' + len(65535) + '\0'

static struct termios g_origTermios = { 0 };
static TermErr g_error = { 0 };

// Initialization

bool termInit(void) {
    if (tcgetattr(STDIN_FILENO, &g_origTermios) != 0) {
        g_error.type = TermErrType_Errno;
        return false;
    }
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
        g_error.type = TermErrType_Errno;
        return false;
    }
    return true;
}

void termQuit(void) {
    // Ignore failures, there is nothing left to do
    (void)tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_origTermios);
}

// Error handling

TermErr *termErr(void) {
    return &g_error;
}

static void printErrMsg(const char *msg, char *desc) {
    if (msg == NULL || *msg == '\0') {
        (void)fprintf(stderr, "%s\r\n", desc);
    } else {
        (void)fprintf(stderr, "%s: %s\r\n", msg, desc);
    }
}

void termLogError(const char *msg) {
    switch (g_error.type) {
    case TermErrType_None:
        printErrMsg(msg, (char *)"no error occurred");
        break;
    case TermErrType_Errno:
        printErrMsg(msg, strerror(errno));
        break;
    default:
        UNREACHABLE;
    }
}

// Input

UcdCP termGetInput(void) {
    UcdCh8 ch = 0;

    if (read(STDIN_FILENO, &ch, 1) < 0) {
        g_error.type = TermErrType_Errno;
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
            g_error.type = TermErrType_Errno;
            return -1;
        }
        if (ch == 0) {
            return chBytes[0];
        }
        chBytes[i] = ch;
    }

    return ucdCh8ToCP(chBytes);
}

// Output

bool termWrite(const void *buf, size_t size) {
    if (write(STDOUT_FILENO, buf, size) == -1) {
        g_error.type = TermErrType_Errno;
        return false;
    }
    return true;
}

bool termSize(size_t *outRows, size_t *outCols) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        g_error.type = TermErrType_Errno;
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

bool termCursorPos(size_t *outX, size_t *outY) {

    char buf[CURSOR_POS_BUF_SIZE];
    if (!termWrite("\033[6n", 4)) {
        goto failure;
    }

    // Read as few characters as possible

    UcdCP ch = termGetInput();
    if (ch < 0) {
        goto failure;
    } else if (ch != '\033') {
        goto failure_msg;
    }

    ch = termGetInput();
    if (ch < 0) {
        goto failure;
    } else if (ch != '[') {
        goto failure_msg;
    }

    // In buf there will be <y>;<x>
    for (size_t i = 0; i < CURSOR_POS_BUF_SIZE; i++) {
        // Going over the buffer size should not be possible considering a max
        // position of 16 bits (like the struct winsize fields).
        if (i == CURSOR_POS_BUF_SIZE - 1) {
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

    if (outX != NULL) {
        *outX = (size_t)(x - 1);
    }
    if (outY != NULL) {
        *outY = (size_t)(y - 1);
    }

    return true;

failure_msg:
    g_error.type = TermErrType_CustomMsg;
    g_error.data.customMsg = "invalid sequence when reading cursor pos";

failure:
    if (outX != NULL) {
        *outX = 0;
    }
    if (outY != NULL) {
        *outY = 0;
    }
    return false;
}
