#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>

#include "nv_term.h"
#include "nv_unicode.h"

#define UNREACHABLE __builtin_unreachable()

static struct termios g_origTermios = { 0 };
static TermErr g_error = { 0 };

// Initialization

bool termInit(void) {
    if (tcgetattr(STDIN_FILENO, &g_origTermios) != 0) {
        g_error.type = TermErrType_errno;
        return false;
    }
    return true;
}

bool termEnableRawMode(uint8_t getKeyTimeoutDSec) {
    struct termios raw = g_origTermios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ISIG | ICANON | IEXTEN);

    if (getKeyTimeoutDSec != 0) {
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = getKeyTimeoutDSec;
    }

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) != 0) {
        g_error.type = TermErrType_errno;
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
    case TermErrType_none:
        printErrMsg(msg, (char *)"no error occurred");
        break;
    case TermErrType_errno:
        printErrMsg(msg, strerror(errno));
        break;
    default:
        UNREACHABLE;
    }
}

// Input

TermKey termGetKey(void) {
    UcdCh8 ch = 0;

    if (read(STDIN_FILENO, &ch, 1) < 0) {
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
        if (read(STDIN_FILENO, outCh, 1) < 0) {
            g_error.type = TermErrType_errno;
            return -1;
        }
        if (ch == 0) {
            return (TermKey)chBytes[0];
        }
        chBytes[i] = ch;
    }

    return (TermKey)ucdCh8ToCh32(chBytes);
}

// Output

bool termWrite(const void *buf, size_t size) {
    if (write(STDOUT_FILENO, buf, size) == -1) {
        g_error.type = TermErrType_errno;
        return false;
    }
    return true;
}
