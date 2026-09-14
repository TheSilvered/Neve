#ifndef NV_KEY_BINDS_H_
#define NV_KEY_BINDS_H_

#include "nv_array.h"
#include "nv_context.h"
#include "nv_term.h"

#define BindAnyKey (TermKey_MAX + 1)

// An array of keys to match against the given command, use `BindAnyKey` to
// allow for any key.
typedef Arr(int32_t) BindSequence;

typedef void (*BindCallback)(
    void *user,
    BindSequence seq,
    CtxSelection selection
);

typedef struct BindMapNode {
    TermKey value;
    BindCallback callback;
    void *userData;
    bool hasSelection;

    uint32_t cap;
    uint32_t len;
    struct BindMapNode *nodes;
} BindMapNode;

typedef struct KeyBind {
    BindSequence sequence; // Sequence of keys to match.
    BindCallback callback; // Action to take when the bind is matched.
    void *userData;        // Additional data to pass to the callback.
    bool hasSelection;     // Whether the callback expects a selection.
} KeyBind;

typedef Arr(KeyBind) KeyBinds;

typedef struct BindMatch {
    bool ambiguous; // *Not used yet*
    KeyBind *bind;  // The matched key bind, may be `NULL`
} BindMatch;

// Find the best matching key bind. If a keybind that matches the 
BindMatch bindMatch(KeyBinds binds, BindSequence sequence);

KeyBinds bindsMakeNormalMode(void);
KeyBinds bindsMakeSelectionMode(void);
KeyBinds bindsMakeEditMode(void);

#endif // !NV_KEY_BINDS_H_
