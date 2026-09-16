#include "nv_key_binds.h"
#include "nv_context.h"
#include "nv_editor.h"
#include "nv_pool.h"
#include "nv_term.h"

#define _minMapCap 4

/* The key bindings are stored in a trie data structure, the only invariant is
 * that any leaf that is not the root of the tree must have a key binding
 * associated with it, this is to simplfiy key bind matching and to avoid
 * having dead branches.
 */

BindMap g_bindRoots = { 0 };
Pool g_bindPool = { 0 };

static BindMap *_mapInsert(BindMap *map, BindMap entry);
static BindMap *_mapGet(BindMap *map, int32_t key);
static bool _mapRemove(BindMap *map, int32_t key);
static BindMatchResult _bindMatchRec(
    BindMap *map,
    int32_t *seq,
    KeyBind *outBind
);

int32_t bindAddRootMap(void) {
    BindMap map = { .key = g_bindRoots.len };
    _mapInsert(&g_bindRoots, map);
    return map.key;
}

bool bindRootMapExists(int32_t id) {
    return _mapGet(&g_bindRoots, id) != NULL;
}

void bindAdd(int32_t root, BindKeys seq, KeyBind keyBind) {
    BindMap *map = _mapGet(&g_bindRoots, root);
    if (map == NULL) return;

    while (*seq != BindEnd) {
        BindMap *newMap = _mapGet(map, *seq);
        if (newMap == NULL) {
            newMap = _mapInsert(map, (BindMap){ .key = *seq });
        }
        map = newMap;
        seq++;
    }

    if (map->value != NULL) {
        poolFree(&g_bindPool, map->value);
    }
    KeyBind *bind = poolAlloc(&g_bindPool);
    *bind = keyBind;
    map->value = bind;
}

bool bindRemove(int32_t root, BindKeys seq) {
    BindMap *map = _mapGet(&g_bindRoots, root);
    if (map == NULL) return false;

    // When deleting a key binding, if it is a leaf node of the tree, the leaf
    // must also be deleted. However this could lead to another leaf that has
    // no binding breaking the invariant of the data structure. To do this we
    // store the last valid node along the sequence along the part of the
    // sequence after it.

    BindMap *lastValidNode = map;
    int32_t branch = *seq;
    while (*seq != BindEnd) {
        map = _mapGet(map, *seq);
        seq++;

        // If the sequence does not exist.
        if (map == NULL) return false;

        if (map->len >= 2 || (map->len == 1 && map->value != NULL)) {
            lastValidNode = map;
            branch = *seq;
        }
    }
    // If the sequence does not end at a key bind.
    if (map->value == NULL) return false;
    poolFree(&g_bindPool, map->value);
    map->value = NULL;
    // If the sequence ends on a valid node 'branch' is equal to BindEnd and
    // nothing is deleted.
    _mapRemove(lastValidNode, branch);
    return true;
}

bool bindExists(int32_t root, BindKeys seq) {
    BindMap *map = _mapGet(&g_bindRoots, root);
    if (map == NULL) return false;
    while (*seq != BindEnd && map != NULL) {
        map = _mapGet(map, *seq);
        seq++;
    }
    return map != NULL && map->value != NULL;
}

BindMatchResult bindMatch(int32_t root, int32_t seq[], KeyBind *outBind) {
    BindMap *rootMap = _mapGet(&g_bindRoots, root);
    if (rootMap == NULL) return BindMatch_NotFound;
    return _bindMatchRec(rootMap, seq, outBind);
}

static BindMatchResult _bindMatchRec(
    BindMap *map,
    int32_t *seq,
    KeyBind *outBind
) {
    // This should not happen but just in case.
    if (seq[0] == BindEnd) return BindMatch_NotFound;
    bool isLast = seq[1] == BindEnd;

    BindMap *wildcard = _mapGet(map, BindAnyKey);
    BindMap *specific = _mapGet(map, seq[0]);

    if (!wildcard && !specific) {
        return BindMatch_NotFound;
    } else if (isLast) {
        // specific = whichever matched, favouring the specific one
        if (!specific) specific = wildcard;
        if (specific->value == NULL) {
            nvAssert(specific->len != 0, "all leaves must have a binding");
            return BindMatch_Incomplete;
        }
        *outBind = *specific->value;
        return specific->len == 0 ? BindMatch_Found : BindMatch_Partial;
    } else if ((wildcard && !specific) || (!wildcard && specific)) {
        return _bindMatchRec(wildcard ? wildcard : specific, seq + 1, outBind);
    }

    // At this point we know that we are not at the end of the sequence and
    // need to explore both branches.

    /*
     * A table that shows the outcome based on the result of exploring the two
     * possible branches. In parenthesis is the key bind taken where it applies.
     *
     *                               Specific
     *                Not F.     Incom.     Found      Part.
     *              +----------+----------+----------+----------+
     *   W   Not F. | Not F.   | Incom.   | Found(S) | Part.(S) |
     *   i          +----------+----------+----------+----------+
     *   l   Incom. | Incom.   | Incom.   | Part.(S) | Part.(S) |
     *   d          +----------+----------+----------+----------+
     *   c   Found  | Found(W) | Part.(W) | Found(S) | Part.(S) |
     *   a          +----------+----------+----------+----------+
     *   r   Part.  | Part.(W) | Part.(W) | Part.(S) | Part.(S) |
     *   d          +----------+----------+----------+----------+
     *
     */

    KeyBind wildBind = { 0 };
    KeyBind specBind = { 0 };

    BindMatchResult wildRes = _bindMatchRec(wildcard, seq + 1, &wildBind);
    BindMatchResult specRes = _bindMatchRec(specific, seq + 1, &specBind);

    *outBind = specRes >= BindMatch_Partial ? specBind : wildBind;
    // If the values lie in the diagonal
    if (wildRes + specRes == BindMatch_Partial) {
        return BindMatch_Partial;
    }
    return nvMax(wildRes, specRes);
}

static int32_t _keyHash(int32_t key) {
    uint32_t ukey = (uint32_t)key;
    // Use some bits from outside the unicode range
    return (int32_t)((ukey >> 21) ^ ukey);
}

static uint16_t _mapIdx(BindMap *nodes, uint16_t cap, int32_t key) {
    int32_t hash = _keyHash(key);
    for (uint16_t i = 0; i < cap - 1; i++) {
        uint16_t idx = (hash + i) & (cap - 1);
        if (nodes[idx].key == TermKey_None || nodes[idx].key == key) {
            return idx;
        }
    }
    return (hash + cap) & (cap - 1);
}

static void _mapDestroy(BindMap *map) {
    if (map->value != NULL) {
        poolFree(&g_bindPool, map->value);
    }
    if (map->nodes == NULL) return;
    for (uint16_t i = 0; i < map->cap; i++) {
        if (map->nodes[i].key != TermKey_None) {
            _mapDestroy(&map->nodes[i]);
        }
    }
    memFree(map->nodes);
}

static void _mapExpand(BindMap *map) {
    uint16_t newCap = nvMax(map->cap * 2, _minMapCap);
    BindMap *newMapNodes = memAllocZeroed(newCap, sizeof(*newMapNodes));

    for (uint16_t i = 0; i < map->cap; i++) {
        if (map->nodes[i].key == TermKey_None) continue;
        uint16_t newIdx = _mapIdx(newMapNodes, newCap, map->nodes[i].key);
        newMapNodes[newIdx] = map->nodes[i];
    }
    memFree(map->nodes);
    map->nodes = newMapNodes;
    map->cap = newCap;
}

static BindMap *_mapInsert(BindMap *map, BindMap entry) {
    nvAssertExpr(map != NULL);
    if (map->len >= (map->cap >> 2) + (map->cap >> 1)) {
        _mapExpand(map);
    }
    uint16_t idx = _mapIdx(map->nodes, map->cap, entry.key);
    if (map->nodes[idx].key != TermKey_None) {
        _mapDestroy(&map->nodes[idx]);
    } else {
        map->len++;
    }
    map->nodes[idx] = entry;
    return &map->nodes[idx];
}

static BindMap *_mapGet(BindMap *map, int32_t key) {
    uint16_t idx = _mapIdx(map->nodes, map->cap, key);
    return map->nodes[idx].key == TermKey_None ? NULL : &map->nodes[idx];
}

static bool _mapRemove(BindMap *map, int32_t key) {
    uint16_t idx = _mapIdx(map->nodes, map->cap, key);
    if (map->nodes[idx].key == TermKey_None) return false;
    _mapDestroy(&map->nodes[idx]);
    uint16_t mask = map->cap - 1;
    idx = (idx + 1) & mask;
    while (
        map->nodes[idx].key != TermKey_None
        && (_keyHash(map->nodes[idx].key) & mask) != idx
    ) {
        uint16_t prevIdx = (idx - 1) & mask;
        map->nodes[prevIdx] = map->nodes[idx];
        map->nodes[idx] = (BindMap){ 0 };
        idx = (idx + 1) & mask;
    }
    return true;
}

void moveCallback(void *user, int32_t *keys, CtxSelection sel) {
    (void)user;
    (void)sel;
    Ctx *ctx = editorActiveContext();

    switch (keys[0]) {
    case 'i':
    case TermKey_ArrowUp:
        ctxCurMoveUp(ctx);
        break;
    case 'I':
        ctxCurMoveToPrevParagraph(ctx);
        break;
    case TermKey_CtrlI: {
        UIBufPanel *panel = editorActivePanel();
        for (uint16_t i = 0; i < panel->elem.h / 2; i++) {
            ctxCurMoveUp(ctx);
        }
        break;
    }
    case 'k':
    case TermKey_ArrowDown:
        ctxCurMoveDown(ctx);
        break;
    case 'K':
        ctxCurMoveToNextParagraph(ctx);
        break;
    case TermKey_CtrlK: {
        UIBufPanel *panel = editorActivePanel();
        for (uint16_t i = 0; i < panel->elem.h / 2; i++) {
            ctxCurMoveDown(ctx);
        }
        break;
    }
    case 'j':
        ctxCurMoveLeft(ctx);
        break;
    case 'J':
        ctxCurMoveToPrevWordStart(ctx);
        break;
    case TermKey_CtrlJ:
        ctxCurMoveToPrevWordEnd(ctx);
        break;
    case 'l':
        ctxCurMoveRight(ctx);
        break;
    case 'L':
        ctxCurMoveToNextWordEnd(ctx);
        break;
    case TermKey_CtrlL:
        ctxCurMoveToNextWordStart(ctx);
        break;
    case 'u':
        ctxCurMoveToLineStart(ctx);
        break;
    case 'U':
        ctxCurMoveToTextStart(ctx);
        break;
    case 'o':
        ctxCurMoveToLineEnd(ctx);
        break;
    case 'O':
        ctxCurMoveToTextEnd(ctx);
        break;
    case TermKey_ArrowLeft:
        ctxCurMoveBack(ctx);
        break;
    case TermKey_ArrowRight:
        ctxCurMoveFwd(ctx);
        break;
    }
}

void editModeCallback(void *user, int32_t *keys, CtxSelection sel) {
    (void)user;
    (void)sel;
    Ctx *ctx = editorActiveContext();

    switch (keys[0]) {
    case TermKey_CtrlA:
        ctxCurMoveToLineStart(ctx);
        break;
    case TermKey_CtrlE:
        ctxCurMoveToLineEnd(ctx);
        break;
    case TermKey_CtrlF:
        ctxCurMoveFwd(ctx);
        break;
    case TermKey_CtrlB:
        ctxCurMoveBack(ctx);
        break;
    case TermKey_CtrlK:
        ctxCurMoveToPrevWordStart(ctx);
        break;
    case TermKey_CtrlL:
        ctxCurMoveToNextWordEnd(ctx);
        break;
    case TermKey_CtrlP:
        ctxCurMoveUp(ctx);
        break;
    case TermKey_CtrlN:
        ctxCurMoveDown(ctx);
        break;
    case TermKey_CtrlZ:
    case TermKey_Backspace:
        ctxRemoveBack(ctx);
        break;
    case TermKey_CtrlX:
    case TermKey_Delete:
        ctxRemoveFwd(ctx);
        break;
    case TermKey_CtrlD:
        ctxIndent(ctx);
        break;
    case TermKey_CtrlS:
        ctxDedent(ctx);
        break;
    case TermKey_CtrlW:
        ctxSelBegin(ctx, false);
        ctxCurMoveToPrevWordStart(ctx);
        ctxSelEnd(ctx);
        if (ctxSelHas(ctx)) {
            ctxRemoveBack(ctx);
        }
        break;
    case TermKey_CtrlR:
        ctxSelBegin(ctx, false);
        ctxCurMoveToNextWordStart(ctx);
        ctxSelEnd(ctx);
        if (ctxSelHas(ctx)) {
            ctxRemoveBack(ctx);
        }
        break;
    case TermKey_CtrlO:
        ctxInsertLineAbove(ctx);
        break;
    case TermKey_CtrlU:
        ctxInsertLineBelow(ctx);
        break;
    case TermKey_CtrlT:
        ctxSelBegin(ctx, false);
        ctxCurMoveToLineStart(ctx);
        ctxSelEnd(ctx);
        if (ctxSelHas(ctx)) {
            ctxRemoveBack(ctx);
        }
        break;
    case TermKey_CtrlY:
        ctxSelBegin(ctx, false);
        ctxCurMoveToLineEnd(ctx);
        ctxSelEnd(ctx);
        if (ctxSelHas(ctx)) {
            ctxRemoveBack(ctx);
        }
        break;
    case TermKey_CtrlC:
    case TermKey_CtrlQ:
    case TermKey_Escape: {
        UIBufPanel *panel = editorActivePanel();
        if (panel != NULL) {
            panel->mode = UIBufMode_Normal;
        }
    }
    }
}

void insertTextCallback(void *user, int32_t *keys, CtxSelection sel) {
    (void)user;
    (void)sel;

    Ctx *ctx = editorActiveContext();
    if (ctx == NULL) return;

    int32_t cp = keys[0];
    if (cp == '\r') {
        cp = '\n';
    }
    if (cp != '\n' || ctx->multiline) {
        ctxInsertCP(ctx, (UcdCP)cp);
    }
}

#define mkBind(map, cb, ...)                                                   \
    bindAdd(map, BindSeq(__VA_ARGS__), (KeyBind) { .callback = (cb) })

#define mkSelBind(map, cb, ...)                                                \
    bindAdd(                                                                   \
        map,                                                                   \
        BindSeq(__VA_ARGS__),                                                  \
        (KeyBind){ .callback = (cb), .hasSel = true }                          \
    )

static void addNormalMovement(int32_t map) {
    mkBind(map, moveCallback, 'i');
    mkBind(map, moveCallback, 'I');
    mkBind(map, moveCallback, 'j');
    mkBind(map, moveCallback, 'J');
    mkBind(map, moveCallback, 'k');
    mkBind(map, moveCallback, 'K');
    mkBind(map, moveCallback, 'l');
    mkBind(map, moveCallback, 'L');
    mkBind(map, moveCallback, 'u');
    mkBind(map, moveCallback, 'U');
    mkBind(map, moveCallback, 'o');
    mkBind(map, moveCallback, 'O');
}

static void addArrowKeys(int32_t map) {
    mkBind(map, moveCallback, TermKey_ArrowLeft);
    mkBind(map, moveCallback, TermKey_ArrowRight);
    mkBind(map, moveCallback, TermKey_ArrowUp);
    mkBind(map, moveCallback, TermKey_ArrowDown);
}

static void bindsMakeNormalMode(void) {
    addNormalMovement(BindMap_Normal);
    addArrowKeys(BindMap_Normal);
}

static void bindsMakeSelectionMode(void) {
    addNormalMovement(BindMap_Selection);
    addArrowKeys(BindMap_Selection);
}

static void bindsMakeEditMode(void) {
    addArrowKeys(BindMap_Edit);

    mkBind(BindMap_Edit, editModeCallback, TermKey_CtrlA);
    mkBind(BindMap_Edit, editModeCallback, TermKey_CtrlE);
    mkBind(BindMap_Edit, editModeCallback, TermKey_CtrlF);
    mkBind(BindMap_Edit, editModeCallback, TermKey_CtrlB);
    mkBind(BindMap_Edit, editModeCallback, TermKey_CtrlK);
    mkBind(BindMap_Edit, editModeCallback, TermKey_CtrlL);
    mkBind(BindMap_Edit, editModeCallback, TermKey_CtrlP);
    mkBind(BindMap_Edit, editModeCallback, TermKey_CtrlN);
    mkBind(BindMap_Edit, editModeCallback, TermKey_CtrlZ);
    mkBind(BindMap_Edit, editModeCallback, TermKey_CtrlX);
    mkBind(BindMap_Edit, editModeCallback, TermKey_CtrlD);
    mkBind(BindMap_Edit, editModeCallback, TermKey_CtrlS);
    mkBind(BindMap_Edit, editModeCallback, TermKey_CtrlW);
    mkBind(BindMap_Edit, editModeCallback, TermKey_CtrlR);
    mkBind(BindMap_Edit, editModeCallback, TermKey_CtrlO);
    mkBind(BindMap_Edit, editModeCallback, TermKey_CtrlU);
    mkBind(BindMap_Edit, editModeCallback, TermKey_CtrlT);
    mkBind(BindMap_Edit, editModeCallback, TermKey_CtrlY);
    mkBind(BindMap_Edit, editModeCallback, TermKey_CtrlC);
    mkBind(BindMap_Edit, editModeCallback, TermKey_CtrlQ);
    mkBind(BindMap_Edit, editModeCallback, TermKey_Escape);
    mkBind(BindMap_Edit, insertTextCallback, BindAnyKey);
}
