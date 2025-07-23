#include "nv_term.h"

#ifdef _WIN32
#include "nv_term_win32.c"
#else
#include "nv_term_unix.c"
#endif // !_WIN32

bool termIsInit(void) {
    return g_initialized;
}

static UcdCP g_keyQueue[4] = { 0 };

static void queueKey(UcdCP key) {
    if (g_keyQueue[0] == 0) {
        g_keyQueue[0] = key;
    } else if (g_keyQueue[1] == 0) {
        g_keyQueue[1] = key;
    } else if (g_keyQueue[2] == 0) {
        g_keyQueue[2] = key;
    } else {
        g_keyQueue[3] = key;
    }
}

static UcdCP getKey(void) {
    if (g_keyQueue[0] != 0) {
        UcdCP key = g_keyQueue[0];
        g_keyQueue[0] = g_keyQueue[1];
        g_keyQueue[1] = g_keyQueue[2];
        g_keyQueue[2] = g_keyQueue[3];
        g_keyQueue[3] = 0;
        return key;
    } else {
        return termGetInput();
    }
}

int termGetKey(void) {
    UcdCP seq[4] = { 0 };
    seq[0] = getKey();
    if (seq[0] != TermKey_Escape) {
        return seq[0];
    }

    seq[1] = getKey();
    if (seq[1] != '[') {
        queueKey(seq[1]);
        return seq[0];
    }

    seq[3] = getKey();
    switch (seq[3]) {
    case 'A': return TermKey_ArrowUp;
    case 'B': return TermKey_ArrowDown;
    case 'C': return TermKey_ArrowRight;
    case 'D': return TermKey_ArrowLeft;
    default:
        queueKey(seq[1]);
        queueKey(seq[2]);
        return seq[0];
    }
}
