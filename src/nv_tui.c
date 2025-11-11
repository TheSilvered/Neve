#include "nv_tui.h"

static bool uiBufPanelKeyHandler_(UIBufPanel *panel, int32_t key);
static bool uiBufHandleNormalMode_(Buf *buf, int32_t key);

void uiBufPanelInit(UIBufPanel *panel, Buf *buf) {
    panel->buf = buf;
    panel->w = 0;
    panel->h = 0;

    panel->keyHandler = uiBufPanelKeyHandler_;
}

static bool uiBufPanelKeyHandler_(UIBufPanel *panel, int32_t key) {
    if (panel->buf->ctx.mode == CtxMode_Normal) {
        return uiBufHandleNormalMode_(panel->buf, key);
    } else {
        return false;
    }
}

static bool uiBufHandleNormalMode_(Buf *buf, int32_t key) {
    switch (key) {
    case 'i':
        ctxCurMoveUp(&buf->ctx);
        break;
    case 'I':
        ctxCurMoveToPrevParagraph(&buf->ctx);
        break;
    case 'k':
        ctxCurMoveDown(&buf->ctx);
        break;
    case 'K':
        ctxCurMoveToNextParagraph(&buf->ctx);
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
        buf->ctx.mode = CtxMode_Edit;
        break;
    case 'E':
        ctxCurMoveToLineEnd(&buf->ctx);
        buf->ctx.mode = CtxMode_Edit;
        break;
    default:
        return false;
    }
    return true;
}
