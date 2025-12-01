#include "nv_tui.h"

static void uiBufPanelUpdater_(UIBufPanel *panel);
static bool uiBufPanelKeyHandler_(UIBufPanel *panel, int32_t key);
static bool uiBufHandleNormalMode_(UIBufPanel *panel, int32_t key);
static bool uiBufHandleEditMode_(UIBufPanel *panel, int32_t key);
static bool uiBufHandleSelectionMode_(UIBufPanel *panel, int32_t key);

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
        return uiBufHandleNormalMode_(panel->buf, key);
    case UIBufMode_Edit:
        // TODO: edit
        return false;
    case UIBufMode_Selection:
        // TODO: selection
        return false;
    default:
        return false;
    }
}

static bool uiBufHandleNormalMode_(UIBufPanel *panel, int32_t key) {
    Buf *buf = panel->buf;
    switch (key) {
    case 'i':
        ctxCurMoveUp(&buf->ctx);
        break;
    case 'I':
        ctxCurMoveToPrevParagraph(&buf->ctx);
        break;
    case TermKey_CtrlI:
        for (uint16_t i = 0; i < panel->w / 2; i++) {
            ctxCurMoveUp(&buf->ctx);
        }
        break;
    case 'k':
        ctxCurMoveDown(&buf->ctx);
        break;
    case 'K':
        ctxCurMoveToNextParagraph(&buf->ctx);
        break;
    case TermKey_CtrlK:
        for (uint16_t i = 0; i < panel->w / 2; i++) {
            ctxCurMoveDown(&buf->ctx);
        }
        break;
    case 'j':
        ctxCurMoveLeft(&buf->ctx);
        break;
    case 'J':
        ctxCurMoveToPrevWordStart(&buf->ctx);
        break;
    case TermKey_CtrlJ:
        ctxCurMoveToPrevWordEnd(&buf->ctx);
        break;
    case 'l':
        ctxCurMoveRight(&buf->ctx);
        break;
    case 'L':
        ctxCurMoveToNextWordEnd(&buf->ctx);
        break;
    case TermKey_CtrlL:
        ctxCurMoveToNextWordStart(&buf->ctx);
        break;
    case 'u':
        ctxCurMoveToLineStart(&buf->ctx);
        break;
    case 'U':
        ctxCurMoveToTextStart(&buf->ctx);
        break;
    case 'o':
        ctxCurMoveToLineEnd(&buf->ctx);
        break;
    case 'O':
        ctxCurMoveToTextEnd(&buf->ctx);
    case 'e':
        panel->mode = UIBufMode_Edit;
        break;
    case 'E':
        ctxCurMoveToLineEnd(&buf->ctx);
        panel->mode = UIBufMode_Edit;
        break;
    case 'h':
        ctxCurMoveToLineEnd(&buf->ctx);
        ctxInsert(&buf->ctx, "\n", 1);
        panel->mode = UIBufMode_Edit;
        break;
    case 'H':
        ctxCurMoveToLineStart(&buf->ctx);
        ctxInsert(&buf->ctx, "\n", 1);
        ctxCurMoveBack(&buf->ctx);
        panel->mode = UIBufMode_Edit;
        break;
    default:
        return false;
    }
    return true;
}

static bool uiBufHandleEditMode_(UIBufPanel *panel, int32_t key) {
    Buf *buf = panel->buf;
    switch (key) {
    case TermKey_CtrlA:
        ctxCurMoveToLineStart(&buf->ctx);
        break;
    case TermKey_CtrlE:
        ctxCurMoveToLineEnd(&buf->ctx);
        break;
    case TermKey_CtrlF:
        ctxCurMoveFwd(&buf->ctx);
        break;
    case TermKey_CtrlB:
        ctxCurMoveBack(&buf->ctx);
        break;
    case TermKey_CtrlP:
        ctxCurMoveUp(&buf->ctx);
        break;
    case TermKey_CtrlN:
        ctxCurMoveDown(&buf->ctx);
        break;
    case TermKey_CtrlH:
        ctxRemoveBack(&buf->ctx);
        break;
    case TermKey_CtrlK:
        ctxRemoveFwd(&buf->ctx);
        break;
    case TermKey_CtrlW:
        ctxSelBegin(&buf->ctx);
        ctxCurMoveToPrevWordStart(&buf->ctx);
        ctxSelEnd(&buf->ctx);
        ctxRemoveBack(&buf->ctx);
        break;
    case TermKey_CtrlR:
        ctxSelBegin(&buf->ctx);
        ctxCurMoveToNextWordEnd(&buf->ctx);
        ctxSelEnd(&buf->ctx);
        ctxRemoveBack(&buf->ctx);
        break;
    case TermKey_CtrlO:
        ctxCurMoveToLineStart(&buf->ctx);
        ctxInsert(&buf->ctx, "\n", 1);
        ctxCurMoveBack(&buf->ctx);
        break;
    case TermKey_CtrlU:
        ctxCurMoveToLineEnd(&buf->ctx);
        ctxInsert(&buf->ctx, "\n", 1);
        break;
    case TermKey_CtrlJ:
        ctxSelBegin(&buf->ctx);
        ctxCurMoveToLineStart(&buf->ctx);
        ctxSelEnd(&buf->ctx);
        ctxRemoveBack(&buf->ctx);
        break;
    case TermKey_CtrlL:
        ctxSelBegin(&buf->ctx);
        ctxCurMoveToLineEnd(&buf->ctx);
        ctxSelEnd(&buf->ctx);
        ctxRemoveBack(&buf->ctx);
        break;
    case TermKey_CtrlQ:
    case TermKey_Escape:
        panel->mode = UIBufMode_Normal;
        break;
    case '\r':
        key = '\n';
        // fallthrough
    default:
        ctxInsertCP(&buf->ctx, (UcdCP)key);
        break;
    }
    return true;
}

static bool uiBufHandleSelectionMode_(UIBufPanel *panel, int32_t key) {

}