#ifndef NV_KEY_BINDS_H_
#define NV_KEY_BINDS_H_

#include "nv_context.h"
#include "nv_term.h"

#define BindAny (TermKey_MAX + 1)
#define BindEnd (TermKey_MAX + 100)

#define BindSeq(...) ((BindKeys){ __VA_ARGS__, BindEnd })

typedef int32_t BindKeys[];
typedef void (*BindCallback)(void *user, BindKeys keys, CtxSelection selection);

typedef struct KeyBind {
    BindCallback callback;
    void *userData;
    bool hasSelection;
} KeyBind;

typedef struct BindMap {
    KeyBind *value;
    int32_t key;

    uint16_t cap;
    uint16_t len;
    struct BindMap *nodes;
} BindMap;

// WARNING: do not change the order of BindMatchResult

typedef enum BindMatchResult {
    // No sequence matches this binding.
    BindMatch_NotFound,
    // The sequence can continue but it currently leads to no binding.
    BindMatch_Incomplete,
    // The sequence unambiguously matches a binding.
    BindMatch_Found,
    // The sequence can continue but it is already a valid binding.
    BindMatch_Partial
} BindMatchResult;

// Note that if two sequences are only distinguished by a wildcard, the one
// without 'BindAny' takes precedence and is reported as found. For example
// say that:
// x,y triggers A
// x,BindAny triggers B
// The sequence x,y will find result in BindMatch_Found with A as the action.

// Default bind roots, added with `bindInit`.
enum BindMapID {
    BindMap_Normal = 1,
    BindMap_Selection,
    BindMap_Edit
};

// Add the default roots.
void bindInit(void);
// Free all memory used by the key bindings.
void bindQuit(void);

// Add a root map for the bindings and return its ID.
int32_t bindAddRootMap(void);
// Check if a root map exists. Using other functions with a non-existent root
// will silently fail.
bool bindRootMapExists(int32_t id);

// Add a key bind, override the existing one if present.
void bindAdd(int32_t root, BindKeys seq, KeyBind keyBind);
// Remove a key bind, return `true` if a key bind was removed and `false` if no
// action was taken. `BindAny` is matched only to itself.
bool bindRemove(int32_t root, BindKeys seq);
// Check if a bind exists.
bool bindExists(int32_t root, BindKeys seq);
// Find the best-match for a sequence.
BindMatchResult bindMatch(int32_t root, BindKeys seq, KeyBind *outBind);

// Print a root tree. Useful for debugging.
void bindPrintRoot(int32_t root);

#endif // !NV_KEY_BINDS_H_
