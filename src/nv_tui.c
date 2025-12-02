#include "nv_tui.h"

static void uiBufPanelUpdater_(UIBufPanel *panel);
static bool uiBufPanelKeyHandler_(UIBufPanel *panel, int32_t key);
static bool uiBufHandleNormalMode_(UIBufPanel *panel, int32_t key);
static bool uiBufHandleEditMode_(UIBufPanel *panel, int32_t key);
static bool uiBufHandleSelectionMode_(UIBufPanel *panel, int32_t key);
static bool uiHandleNormalMovement_(UIBufPanel *panel, int32_t key);
static bool uiHandleArrowKeys_(UIBufPanel *panel, int32_t key);

void uiBufPanelInit(UIBufPanel *panel, Buf *buf) {
panel->buf = buf;
panel->w = 0;
    panel->h = 0;

    panel->keyHandler = uiBufPanelKeyHandler_;
    panel->updater = uiBufPanelUpdater_;
}

static void uiBufPanelUpdater_(UIBufPanel *panel) {
    Ctx *ctx = &panel->buf->ctx;
    size_t lines = ctxLineCount(ctx);

    if (ctx->cursors.len != 1) {
        if (panel->scrollY > lines) {
            panel->scrollY = lines - 1;
        }
        return;
    }

    size_t line, col;
    ctxPosAt(ctx, ctx->cursors.items[0].idx, &line, &col);
    if (panel->scrollX > col) {
        panel->scrollX = col;
    } else if (panel->scrollX + panel->w < col) {
        panel->scrollX = col - panel->w;
    }

    if (panel->scrollY > line) {
        panel->scrollY = line;
    } else if (panel->scrollY + panel->h < line) {
        panel->scrollY = line - panel->h;
    }
}

static bool uiBufPanelKeyHandler_(UIBufPanel *panel, int32_t key) {
    switch (panel->mode) {
    case UIBufMode_Normal:
        return uiBufHandleNormalMode_(panel, key);
    case UIBufMode_Edit:
        return uiBufHandleEditMode_(panel, key);
    case UIBufMode_Selection:
        return uiBufHandleSelectionMode_(panel, key);
    default:
        return false;
    }
}

static bool uiHandleNormalMovement_(UIBufPanel *panel, int32_t key) {
    Ctx *ctx = &panel->buf->ctx;
    switch (key) {
    case 'i':
        ctxCurMoveUp(ctx);
        break;
    case 'I':
        ctxCurMoveToPrevParagraph(ctx);
        break;
    case TermKey_CtrlI:
        for (uint16_t i = 0; i < panel->w / 2; i++) {
            ctxCurMoveUp(ctx);
        }
        break;
    case 'k':
        ctxCurMoveDown(ctx);
        break;
    case 'K':
        ctxCurMoveToNextParagraph(ctx);
        break;
    case TermKey_CtrlK:
        for (uint16_t i = 0; i < panel->w / 2; i++) {
            ctxCurMoveDown(ctx);
        }
        break;
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
    default:
        return false;
    }
    return true;
}

static bool uiHandleArrowKeys_(UIBufPanel *panel, int32_t key) {
    Ctx *ctx = &panel->buf->ctx;
    switch (key) {
    case TermKey_ArrowDown:
        ctxCurMoveDown(ctx);
        break;
    case TermKey_ArrowUp:
        ctxCurMoveUp(ctx);
        break;
    case TermKey_ArrowLeft:
        ctxCurMoveBack(ctx);
        break;
    case TermKey_ArrowRight:
        ctxCurMoveFwd(ctx);
        break;
    default:
        return false;
    }
    return true;
}

static bool uiBufHandleNormalMode_(UIBufPanel *panel, int32_t key) {
    Ctx *ctx = &panel->buf->ctx;
    if (uiHandleNormalMovement_(panel, key) || uiHandleArrowKeys_(panel, key)) {
        return true;
    }
    switch (key) {
    case 'e':
        panel->mode = UIBufMode_Edit;
        break;
    case 'E':
        ctxCurMoveToLineEnd(ctx);
        panel->mode = UIBufMode_Edit;
        break;
    case 'h':
        ctxInsertLineBelow(ctx);
        panel->mode = UIBufMode_Edit;
        break;
    case 'H':
        ctxInsertLineAbove(ctx);
        panel->mode = UIBufMode_Edit;
        break;
    case 's':
        ctxSelBegin(ctx);
        panel->mode = UIBufMode_Selection;
        break;
    case 'Y':
        ctxCurMoveToLineStart(ctx);
        ctxSelBegin(ctx);
        ctxCurMoveToLineEnd(ctx);
        ctxSelEnd(ctx);
        if (ctxSelHas(ctx)) {
            ctxRemoveBack(ctx);
        }
        break;
    case TermKey_CtrlY:
        ctxSelBegin(ctx);
        ctxCurMoveToLineEnd(ctx);
        ctxSelEnd(ctx);
        if (ctxSelHas(ctx)) {
            ctxRemoveBack(ctx);
        }
        break;
    case 'q':
        ctxRemoveBack(ctx);
        break;
    case 'Q':
        ctxRemoveFwd(ctx);
        break;
    case 'R':
        ctxCurMoveToLineStart(ctx);
        ctxSelBegin(ctx);
        ctxCurMoveToLineEnd(ctx);
        ctxSelEnd(ctx);
        if (ctxSelHas(ctx)) {
            ctxRemoveBack(ctx);
        }
        panel->mode = UIBufMode_Edit;
        break;
    case TermKey_CtrlR:
        ctxSelBegin(ctx);
        ctxCurMoveToLineEnd(ctx);
        ctxSelEnd(ctx);
        if (ctxSelHas(ctx)) {
            ctxRemoveBack(ctx);
        }
        break;
    default:
        return false;
    }
    return true;
}

static bool uiBufHandleEditMode_(UIBufPanel *panel, int32_t key) {
    Ctx *ctx = &panel->buf->ctx;
    if (uiHandleArrowKeys_(panel, key)) {
        return true;
    }
    switch (key) {
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
    case TermKey_CtrlW:
        ctxSelBegin(ctx);
        ctxCurMoveToPrevWordStart(ctx);
        ctxSelEnd(ctx);
        ctxRemoveBack(ctx);
        break;
    case TermKey_CtrlR:
        ctxSelBegin(ctx);
        ctxCurMoveToNextWordEnd(ctx);
        ctxSelEnd(ctx);
        ctxRemoveBack(ctx);
        break;
    case TermKey_CtrlO:
        ctxInsertLineAbove(ctx);
        break;
    case TermKey_CtrlU:
        ctxInsertLineBelow(ctx);
        break;
    case TermKey_CtrlT:
        ctxSelBegin(ctx);
        ctxCurMoveToLineStart(ctx);
        ctxSelEnd(ctx);
        ctxRemoveBack(ctx);
        break;
    case TermKey_CtrlY:
        ctxSelBegin(ctx);
        ctxCurMoveToLineEnd(ctx);
        ctxSelEnd(ctx);
        ctxRemoveBack(ctx);
        break;
    case TermKey_CtrlQ:
    case TermKey_Escape:
        panel->mode = UIBufMode_Normal;
        break;
    case '\r':
        key = '\n';
        // fallthrough
    default:
        ctxInsertCP(ctx, (UcdCP)key);
        break;
    }
    return true;
}

static bool uiBufHandleSelectionMode_(UIBufPanel *panel, int32_t key) {
    Ctx *ctx = &panel->buf->ctx;
    if (uiHandleNormalMovement_(panel, key) || uiHandleArrowKeys_(panel, key)) {
        return true;
    }
    switch (key) {
    case 'y':
        ctxSelEnd(ctx);
        if (ctxSelHas(ctx)) {
            ctxRemoveBack(ctx);
        }
        panel->mode = UIBufMode_Normal;
        break;
    case 'r':
        ctxSelEnd(ctx);
        if (ctxSelHas(ctx)) {
            ctxRemoveBack(ctx);
        }
        panel->mode = UIBufMode_Edit;
        break;
    case 'h':
        if (ctxSelIsActive(ctx)) {
            ctxSelEnd(ctx);
        } else {
            ctxSelBegin(ctx);
        }
        break;
    case TermKey_CtrlQ:
    case TermKey_Escape:
        ctxSelCancel(ctx);
        panel->mode = UIBufMode_Normal;
        break;
    default:
        return false;
    }
    return true;
}
