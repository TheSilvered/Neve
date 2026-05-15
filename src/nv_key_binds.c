#include "nv_key_binds.h"

static int32_t getScore(KeyBind *bind, BindSequence seq, bool *ambiguous);

BindMatch bindMatch(KeyBinds binds, BindSequence sequence) {
    KeyBind *best = NULL;
    // Number of 'any' characters matched
    int32_t bestAnyCount = sequence.len;
    bool ambiguous = false;

    for (size_t i = 0; i < binds.len; i++) {
        int32_t anyCount = getScore(&binds.items[i], sequence, &ambiguous);
        if (anyCount >= 0 && anyCount <= bestAnyCount) {
            best = &binds.items[i];
            bestAnyCount = anyCount;
        }
    }
    return (BindMatch) {
        .bind = best,
        .ambiguous = best == NULL ? false : ambiguous
    };
}

static int32_t getScore(KeyBind *bind, BindSequence seq, bool *ambiguous) {
    if (
        bind->sequence.len < seq.len
        || (*ambiguous && bind->sequence.len != seq.len)
    ) {
        return -1;
    }

    int32_t anyCount = 0;
    for (size_t i = 0; i < seq.len; i++) {
        TermKey bindKey = bind->sequence.items[i];
        if (bindKey == BindAnyKey) {
            anyCount++;
        } else if (bindKey != seq.len) {
            return -1;
        }
    }
    return anyCount;
}
