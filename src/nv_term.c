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

// Read a possible escape sequence from the terminal.
// Return the number of elements written to outBuf.
// If no character is available return 0.
// If there is only a simple character return 1 and the character is in the
// first slot of outBuf.
// Otherwise parse an escape sequence. The first and last filled slots are the
// First and last characters of the escape sequence. Any number in the mmiddle
// Is an already parsed arguent (the numbers are already integers instead of
// being strings of digit characters).
static size_t readEscapeSeq_(int32_t *outBuf, size_t bufSize) {
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

    size_t i = 1;
    size_t numIdx = 0;
    char numBuf[6] = { 0 };
    for (;;) {
        key = getKey_();

        // Ignore any incomplete sequences
        if (key == TermKey_None) {
            return 0;
        }

        // Add digits to numBuf to be parsed later
        if (key >= '0' && key <= '9') {
            if (numIdx < sizeof(numBuf) - 1) {
                numBuf[numIdx++] = key;
            }
            continue;
        }

        // When the digits end parse the number
        numBuf[numIdx] = '\0';
        unsigned long num = strtoul(numBuf, NULL, 10);
        memset(numBuf, 0, sizeof(numBuf));

        if (num == 0) {
            num = 1;
        } else if (num > INT16_MAX) {
            num = INT16_MAX;
        }

        if (i < bufSize) {
            outBuf[i++] = num;
        }

        // Expect more arguments if there is a semicolon
        if (key == ';') {
            continue;
        }

        // Otherwise add the last character of the sequence to outBuf
        if (i < bufSize) {
            outBuf[i++] = key;
        }
        // No early return for sequences that are too long for the buffer,
        // this prevents any extra characters to be interpreted as keystrokes
        // by themselves instead of as being part of an escape sequence.

        break;
    }

    // Ignore sequences too long for the buffer
    return i < bufSize ? i : 0;
}

int32_t termGetKey(void) {
#define SeqBufLen_ 32
    int32_t seq[SeqBufLen_] = { 0 };
    size_t seqLen = readEscapeSeq_(seq, SeqBufLen_);
#undef SeqBufLen_

    if (seqLen == 0) {
        return 0;
    } else if (seqLen == 1) {
        return seq[0];
    }

    if (seq[0] == 'O') {
        if (seqLen != 2) {
            return 0;
        }
        switch (seq[1]) {
        case 'A': return TermKey_ArrowUp;
        case 'B': return TermKey_ArrowDown;
        case 'C': return TermKey_ArrowRight;
        case 'D': return TermKey_ArrowLeft;
        case 'F': return TermKey_End;
        case 'H': return TermKey_Home;
        case 'P': return TermKey_F1;
        case 'Q': return TermKey_F2;
        case 'R': return TermKey_F3;
        case 'S': return TermKey_F4;
        case 't': return TermKey_F5;
        case 'u': return TermKey_F6;
        case 'v': return TermKey_F7;
        case 'l': return TermKey_F8;
        case 'w': return TermKey_F9;
        case 'x': return TermKey_F10;
        default: return 0;
        }
    }
    assert(seq[0] == '[');
    switch (seq[seqLen - 1]) {
    case 'A': return TermKey_ArrowUp;
    case 'B': return TermKey_ArrowDown;
    case 'C': return TermKey_ArrowRight;
    case 'D': return TermKey_ArrowLeft;
    case 'H': return TermKey_Home;
    case 'F': return TermKey_End;
    case '~':
        if (seqLen < 2) {
            return 0 ;
        }
        switch (seq[1]) {
            case 1:
            case 7:
                return TermKey_Home;
            case 3:
                return TermKey_Delete;
            case 4:
                return TermKey_End;
            case 11: return TermKey_F1;
            case 12: return TermKey_F2;
            case 13: return TermKey_F3;
            case 14: return TermKey_F4;
            case 15: return TermKey_F5;
            case 17: return TermKey_F6;
            case 18: return TermKey_F7;
            case 19: return TermKey_F8;
            case 20: return TermKey_F9;
            case 21: return TermKey_F10;
            case 23: return TermKey_F11;
            case 24: return TermKey_F12;
            default: return 0;
        }
    default: return 0;
    }
}
