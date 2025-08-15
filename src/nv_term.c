#include <assert.h>
#include "nv_term.h"

#ifdef _WIN32
#include "nv_term_win32.c"
#else
#include "nv_term_unix.c"
#endif // !_WIN32

bool termIsInit(void) {
    return g_initialized;
}

static UcdCP g_queuedKey = 0;

static UcdCP getKey_(void) {
    if (g_queuedKey != 0) {
        UcdCP key = g_queuedKey;
        g_queuedKey = 0;
        return key;
    } else {
        return termGetInput();
    }
}

static size_t readEscapeSeq_(UcdCP *outBuf, size_t bufSize) {
    assert(bufSize >= 3);
    UcdCP key = getKey_();
    if (key == TermKey_None) {
        return 0;
    }

    if (key != TermKey_Escape) {
        outBuf[0] = key;
        return 1;
    }

    key = getKey_();
    if (key == TermKey_Escape) {
        // ESC ESC is the same as a single ESC
        outBuf[0] = key;
        return 1;
    }
    if (key != '[' && key != 'O') {
        outBuf[0] = TermKey_Escape;
        g_queuedKey = key;
        return 1;
    }
    // The initial ESC is omitted in the buffer.
    outBuf[0] = key;

    if (key == 'O') {
        key = getKey_();
        if (key == TermKey_None) {
            return 0;
        }
        outBuf[1] = key;
        return 2;
    }

    size_t i = 1;
    for (;;) {
        key = getKey_();
        if (i < bufSize) {
            outBuf[i++] = key;
        }

        if (key == TermKey_None) {
            return 0;
        }

        if ((key >= '0' && key <= '9') || key == ';') {
            continue;
        }
    }

    // Ignore sequences too long for the buffer
    return i < bufSize ? i : 0;
}

int termGetKey(void) {
#define SeqBufLen_ 32
    UcdCP seq[SeqBufLen_] = { 0 };
    size_t seqLen = readEscapeSeq_(seq, SeqBufLen_);
#undef SeqBufLen_

    if (seqLen == 0) {
        return 0;
    } else if (seqLen == 1) {
        return seq[0];
    }

    if (seq[0] == 'O') {
        switch (seq[1]) {
        case 'A': return TermKey_ArrowUp;
        case 'B': return TermKey_ArrowDown;
        case 'C': return TermKey_ArrowRight;
        case 'D': return TermKey_ArrowLeft;
        default: return 0;
        }
    }
    assert(seq[0] == '[');
    switch (seq[1]) {
    case 'A': return TermKey_ArrowUp;
    case 'B': return TermKey_ArrowDown;
    case 'C': return TermKey_ArrowRight;
    case 'D': return TermKey_ArrowLeft;
    default: return 0;
    }
}
