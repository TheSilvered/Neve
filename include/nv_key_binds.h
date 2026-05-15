#ifndef NV_KEY_BINDS_H_
#define NV_KEY_BINDS_H_

#include "nv_array.h"
#include "nv_context.h"
#include "nv_term.h"

#define BindAnyKey (TermKey_MAX + 1)

// An array of keys to match agains the given command.
// All keys are matched exactly except `TermKey_BindAny` that allows all keys
// and `TermKey_BindSelection` that expects an immediate selection.
typedef Arr(TermKey) BindSequence;

typedef void (*BindCallback)(
    void *user,
    BindSequence seq,
    CtxSelection selection
);

typedef struct KeyBind {
    BindSequence sequence;
    BindCallback callback;
    void *userData;
} KeyBind;

typedef Arr(KeyBind) KeyBinds;

typedef struct BindMatch {
    bool ambiguous;
    KeyBind *bind;
} BindMatch;

BindMatch bindMatch(KeyBinds binds, BindSequence sequence);

#endif // !NV_KEY_BINDS_H_
