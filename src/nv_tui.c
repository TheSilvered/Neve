#include "nv_editor.h"
#include "nv_tui.h"

static void _uiElemInit(
    UIElement *elem,
    int16_t x,
    int16_t y,
    uint16_t w,
    uint16_t h,
    UIKeyHandler keyHandler,
    UIUpdater updater
);
static void _uiBufPanelUpdater(UIBufPanel *panel);
static bool _uiBufPanelKeyHandler(UIBufPanel *panel, int32_t key);
static bool _uiBufHandleNormalMode(UIBufPanel *panel, int32_t key);
static bool _uiBufHandleEditMode(UIBufPanel *panel, int32_t key);
static bool _uiBufHandleSelectionMode(UIBufPanel *panel, int32_t key);
static bool _uiHandleNormalMovement(UIBufPanel *panel, int32_t key);
static bool _uiHandleArrowKeys(UIBufPanel *panel, int32_t key);

static void _uiElemInit(
    UIElement *elem,
    int16_t x,
    int16_t y,
    uint16_t w,
    uint16_t h,
    UIKeyHandler keyHandler,
    UIUpdater updater
) {
    elem->x = x;
    elem->y = y;
    elem->w = w;
    elem->h = h;
    elem->keyHandler = keyHandler;
    elem->updater = updater;
}

void uiUpdate(UIElement *elem) {
    elem->updater(elem);
}

bool uiHandleKey(UIElement *elem, int32_t key) {
    return elem->keyHandler(elem, key);
}

void uiBufPanelInit(UIBufPanel *panel, BufHandle bufHd) {
    _uiElemInit(
        &panel->elem,
        0, 0, 0, 0,
        (UIKeyHandler)_uiBufPanelKeyHandler,
        (UIUpdater)_uiBufPanelUpdater
    );

    panel->bufHd = bufHd;
    panel->mode = UIBufMode_Normal;
    panel->scrollX = 0;
    panel->scrollY = 0;
}

static void _uiBufPanelUpdater(UIBufPanel *panel) {
    Buf *buf = bufRef(&g_ed.buffers, panel->bufHd);
    if (buf == NULL) {
        return;
    }

    Ctx *ctx = &buf->ctx;
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
    } else if (panel->scrollX + panel->elem.w <= col) {
        panel->scrollX = col - panel->elem.w + 1;
    }

    if (panel->scrollY > line) {
        panel->scrollY = line;
    } else if (panel->scrollY + panel->elem.h <= line) {
        panel->scrollY = line - panel->elem.h + 1;
    }
}

static bool _uiBufPanelKeyHandler(UIBufPanel *panel, int32_t key) {
    switch (panel->mode) {
    case UIBufMode_Normal:
        return _uiBufHandleNormalMode(panel, key);
    case UIBufMode_Edit:
        return _uiBufHandleEditMode(panel, key);
    case UIBufMode_Selection:
        return _uiBufHandleSelectionMode(panel, key);
    default:
        return false;
    }
}

static bool _uiHandleNormalMovement(UIBufPanel *panel, int32_t key) {
    Buf *buf = bufRef(&g_ed.buffers, panel->bufHd);
    if (buf == NULL) {
        return false;
    }

    Ctx *ctx = &buf->ctx;
    switch (key) {
    case 'i':
        ctxCurMoveUp(ctx);
        break;
    case 'I':
        ctxCurMoveToPrevParagraph(ctx);
        break;
    case TermKey_CtrlI:
        for (uint16_t i = 0; i < panel->elem.w / 2; i++) {
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
        for (uint16_t i = 0; i < panel->elem.w / 2; i++) {
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

static bool _uiHandleArrowKeys(UIBufPanel *panel, int32_t key) {
    Buf *buf = bufRef(&g_ed.buffers, panel->bufHd);
    if (buf == NULL) {
        return false;
    }

    Ctx *ctx = &buf->ctx;
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

static bool _uiBufHandleNormalMode(UIBufPanel *panel, int32_t key) {
    Buf *buf = bufRef(&g_ed.buffers, panel->bufHd);
    if (buf == NULL) {
        return false;
    }

    Ctx *ctx = &buf->ctx;
    if (_uiHandleNormalMovement(panel, key) || _uiHandleArrowKeys(panel, key)) {
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

static bool _uiBufHandleEditMode(UIBufPanel *panel, int32_t key) {
    Buf *buf = bufRef(&g_ed.buffers, panel->bufHd);
    if (buf == NULL) {
        return false;
    }

    Ctx *ctx = &buf->ctx;
    if (_uiHandleArrowKeys(panel, key)) {
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

static bool _uiBufHandleSelectionMode(UIBufPanel *panel, int32_t key) {
    Buf *buf = bufRef(&g_ed.buffers, panel->bufHd);
    if (buf == NULL) {
        return false;
    }

    Ctx *ctx = &buf->ctx;
    if (_uiHandleNormalMovement(panel, key) || _uiHandleArrowKeys(panel, key)) {
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
