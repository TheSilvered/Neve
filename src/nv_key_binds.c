#include "nv_key_binds.h"
#include "nv_context.h"
#include "nv_editor.h"
#include "nv_term.h"

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
        int32_t bindKey = bind->sequence.items[i];
        if (bindKey == BindAnyKey) {
            anyCount++;
        } else if (bindKey != seq.items[i]) {
            return -1;
        }
    }
    return anyCount;
}

void moveCallback(void *user, BindSequence seq, CtxSelection sel) {
    (void)user;
    (void)sel;
    Ctx *ctx = editorActiveContext();

    switch (seq.items[0]) {
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

void editModeCallback(void *user, BindSequence seq, CtxSelection sel) {
    (void)user;
    (void)sel;
    Ctx *ctx = editorActiveContext();

    switch (seq.items[0]) {
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

void insertTextCallback(void *user, BindSequence seq, CtxSelection sel) {
    (void)user;
    (void)sel;

    Ctx *ctx = editorActiveContext();
    if (ctx == NULL) return;

    int32_t cp = seq.items[0];
    if (cp == '\r') {
        cp = '\n';
    }
    if (cp != '\n' || ctx->multiline) {
        ctxInsertCP(ctx, (UcdCP)cp);
    }
}

static void addBind(
    KeyBinds *binds,
    BindCallback callback,
    int32_t *seq,
    size_t count,
    bool hasSel
) {
    BindSequence seqArr = { 0 };
    arrAppendMany(&seqArr, seq, count);
    arrAppend(binds, (KeyBind){
        .sequence = seqArr,
        .callback = callback,
        .hasSelection = hasSel,
        .userData = NULL
    });
}

#define mkBind(binds, callback, ...)                                           \
    addBind(                                                                   \
        (binds),                                                               \
        (callback),                                                            \
        (int32_t[]){__VA_ARGS__},                                              \
        sizeof((int32_t[]){__VA_ARGS__}) / sizeof(TermKey),                    \
        false                                                                  \
    )
#define mkSelBind(binds, callback, ...) \
    addBind(                                                                   \
        (binds),                                                               \
        (callback),                                                            \
        (int32_t[]){__VA_ARGS__},                                              \
        sizeof((int32_t[]){__VA_ARGS__}) / sizeof(TermKey),                    \
        true                                                                   \
    )

static void addNormalMovement(KeyBinds *binds) {
    mkBind(binds, moveCallback, 'i');
    mkBind(binds, moveCallback, 'I');
    mkBind(binds, moveCallback, 'j');
    mkBind(binds, moveCallback, 'J');
    mkBind(binds, moveCallback, 'k');
    mkBind(binds, moveCallback, 'K');
    mkBind(binds, moveCallback, 'l');
    mkBind(binds, moveCallback, 'L');
    mkBind(binds, moveCallback, 'u');
    mkBind(binds, moveCallback, 'U');
    mkBind(binds, moveCallback, 'o');
    mkBind(binds, moveCallback, 'O');
}

static void addArrowKeys(KeyBinds *binds) {
    mkBind(binds, moveCallback, TermKey_ArrowLeft);
    mkBind(binds, moveCallback, TermKey_ArrowRight);
    mkBind(binds, moveCallback, TermKey_ArrowUp);
    mkBind(binds, moveCallback, TermKey_ArrowDown);
}

KeyBinds bindsMakeNormalMode(void) {
    KeyBinds binds = { 0 };
    addNormalMovement(&binds);
    addArrowKeys(&binds);

    return binds;
}

KeyBinds bindsMakeSelectionMode(void) {
    KeyBinds binds = { 0 };
    addNormalMovement(&binds);
    addArrowKeys(&binds);

    return binds;
}

KeyBinds bindsMakeEditMode(void) {
    KeyBinds binds = { 0 };
    addArrowKeys(&binds);

    mkBind(&binds, editModeCallback, TermKey_CtrlA);
    mkBind(&binds, editModeCallback, TermKey_CtrlE);
    mkBind(&binds, editModeCallback, TermKey_CtrlF);
    mkBind(&binds, editModeCallback, TermKey_CtrlB);
    mkBind(&binds, editModeCallback, TermKey_CtrlK);
    mkBind(&binds, editModeCallback, TermKey_CtrlL);
    mkBind(&binds, editModeCallback, TermKey_CtrlP);
    mkBind(&binds, editModeCallback, TermKey_CtrlN);
    mkBind(&binds, editModeCallback, TermKey_CtrlZ);
    mkBind(&binds, editModeCallback, TermKey_CtrlX);
    mkBind(&binds, editModeCallback, TermKey_CtrlD);
    mkBind(&binds, editModeCallback, TermKey_CtrlS);
    mkBind(&binds, editModeCallback, TermKey_CtrlW);
    mkBind(&binds, editModeCallback, TermKey_CtrlR);
    mkBind(&binds, editModeCallback, TermKey_CtrlO);
    mkBind(&binds, editModeCallback, TermKey_CtrlU);
    mkBind(&binds, editModeCallback, TermKey_CtrlT);
    mkBind(&binds, editModeCallback, TermKey_CtrlY);
    mkBind(&binds, editModeCallback, TermKey_CtrlC);
    mkBind(&binds, editModeCallback, TermKey_CtrlQ);
    mkBind(&binds, editModeCallback, TermKey_Escape);
    mkBind(&binds, insertTextCallback, BindAnyKey);

    return binds;
}
