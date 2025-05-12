#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>

#include "nv_term.h"

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

bool termEnableRawMode(void) {
    struct termios raw = g_origTermios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ISIG | ICANON | IEXTEN);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

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

static TermKey readFullKey(unsigned char ch0, size_t chLen) {
    TermKey key = ch0 & ((1 << (6 - chLen)) - 1);
    for (size_t i = 0; i < chLen; i++) {
        unsigned char ch = 0;
        if (read(STDIN_FILENO, &ch, 1) < 0) {
            g_error.type = TermErrType_errno;
            return -1;
        }
        if (ch == 0 || (ch & 0xc0) != 0x80)
            return key;
        key = (key << 6) | (ch & 0x3f);
    }
    return key;
}

TermKey termGetKey(void) {
    unsigned char ch = 0;
    if (read(STDIN_FILENO, &ch, 1) < 0) {
        g_error.type = TermErrType_errno;
        return -1;
    }

    if (ch < 128)
        return (TermKey)ch;

    // Read the full UTF-8 character
    if ((ch & 0xe0) == 0xc0) {
        return readFullKey(ch, 1);
    } else if ((ch & 0xf0) == 0xe0) {
        return readFullKey(ch, 2);
    } else if ((ch & 0xf8) == 0xf0) {
        return readFullKey(ch, 3);
    } else {
        return (TermKey)ch;
    }
}
