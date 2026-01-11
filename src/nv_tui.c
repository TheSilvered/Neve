#include <math.h>
#include "nv_editor.h"
#include "nv_tui.h"

static void _uiElemInit(
    UIElement *elem,
    UIKeyHandler keyHandler,
    UIUpdater updater
);

static void _uiUpdater(UI *ui);
static bool _uiKeyHandler(UI *ui, int32_t key);
static void _uiBufPanelUpdater(UIBufPanel *panel);
static bool _uiBufPanelKeyHandler(UIBufPanel *panel, int32_t key);
static bool _uiBufHandleNormalMode(UIBufPanel *panel, int32_t key);
static bool _uiBufHandleEditMode(UIBufPanel *panel, int32_t key);
static bool _uiBufHandleSelectionMode(UIBufPanel *panel, int32_t key);
static bool _uiHandleNormalMovement(UIBufPanel *panel, int32_t key);

// Edit always handles the input.
// Return false if edit mode should be exited otherwise return true.
static bool _uiHandleEditMode(Ctx *ctx, int32_t key);
static bool _uiHandleArrowKeys(Ctx *ctx, int32_t key);

static void _uiCmdInputUpdater(UICmdInput *cmdInput);
static bool _uiCmdInputKeyHandler(UICmdInput *cmdInput, int32_t key);

static void _uiElemInit(
    UIElement *elem,
    UIKeyHandler keyHandler,
    UIUpdater updater
) {
    elem->x = 0;
    elem->y = 0;
    elem->w = 0;
    elem->h = 0;
    elem->keyHandler = keyHandler;
    elem->updater = updater;
}

void uiUpdate(UIElement *elem) {
    if (elem->updater != NULL) {
        elem->updater(elem);
    }
}

bool uiHandleKey(UIElement *elem, int32_t key) {
    if (elem == NULL) {
        return false;
    } else if (elem->keyHandler != NULL) {
        return elem->keyHandler(elem, key);
    } else {
        return false;
    }
}

void uiInit(UI *ui) {
    _uiElemInit(&ui->elem, (UIKeyHandler)_uiKeyHandler, (UIUpdater)_uiUpdater);
    _uiElemInit(&ui->statusBar, NULL, NULL);
    uiBufPanelInit(&ui->bufPanel);
    uiCmdInputInit(&ui->cmdInput);
}

void uiResize(UI *ui, uint16_t w, uint16_t h) {
    ui->elem.w = w;
    ui->elem.h = h;
}

void uiBufPanelInit(UIBufPanel *panel) {
    _uiElemInit(
        &panel->elem,
        (UIKeyHandler)_uiBufPanelKeyHandler,
        (UIUpdater)_uiBufPanelUpdater
    );

    panel->bufHd = bufInvalidHandle;
    panel->mode = UIBufMode_Normal;
    panel->scrollX = 0;
    panel->scrollY = 0;
}

void uiCmdInputInit(UICmdInput *cmdInput) {
    _uiElemInit(
        &cmdInput->elem,
        (UIKeyHandler)_uiCmdInputKeyHandler,
        (UIUpdater)_uiCmdInputUpdater
    );
    ctxInit(&cmdInput->ctx, false);
    cmdInput->state = UICmdInput_Canceled;
}

static void _uiUpdater(UI *ui) {
    if (ui->elem.h == 0) {
        return;
    }

    ui->bufPanel.elem.w = ui->elem.w;
    if (ui->cmdInput.state == UICmdInput_Inserting) {
        ui->bufPanel.elem.h = ui->elem.h - 2;
    } else {
        ui->bufPanel.elem.h = ui->elem.h - 1;
    }

    ui->cmdInput.elem.w = ui->elem.w;
    ui->cmdInput.elem.h = 1;
    ui->cmdInput.elem.y = ui->elem.h - 2;

    ui->statusBar.w = ui->elem.w;
    ui->statusBar.h = 1;
    ui->statusBar.y = ui->elem.h - 1;

    uiUpdate(&ui->bufPanel.elem);
    uiUpdate(&ui->cmdInput.elem);
    uiUpdate(&ui->statusBar);
}

static bool _uiKeyHandler(UI *ui, int32_t key) {
    if (ui->cmdInput.state == UICmdInput_Inserting) {
        if (uiHandleKey(&ui->cmdInput.elem, key)) {
            return true;
        }
    } else if (uiHandleKey(&ui->bufPanel.elem, key)) {
        return true;
    }

    switch (key) {
    case TermKey_CtrlQ:
        g_ed.running = false;
        return true;
    case 'p':
        editorOpenCommandPalette();
        return true;
    }

    return false;
}

static void _uiBufPanelUpdater(UIBufPanel *panel) {
    Buf *buf = bufRef(&g_ed.buffers, panel->bufHd);
    if (buf == NULL) {
        return;
    }

    Ctx *ctx = &buf->ctx;
    size_t lines = ctxLineCount(ctx);
    // log10(lines) + 1 is the width of the number, +1 for a space after
    uint8_t numColWidth = (uint8_t)log10((double)lines) + 2;

    if (panel->scrollY > lines) {
        panel->scrollY = lines - 1;
    }

    if (ctx->cursors.len != 1) {
        return;
    }

    size_t line, col;
    ctxPosAt(ctx, ctx->cursors.items[0].idx, &line, &col);

    if (panel->scrollX > col) {
        panel->scrollX = col;
    } else if (panel->scrollX + panel->elem.w - numColWidth <= col) {
        panel->scrollX = col - panel->elem.w + numColWidth + 1;
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


static void _uiCmdInputUpdater(UICmdInput *cmdInput) {
    if (cmdInput->ctx.cursors.len != 1) {
        return;
    }

    size_t col;
    ctxPosAt(&cmdInput->ctx, cmdInput->ctx.cursors.items[0].idx, NULL, &col);

    if (cmdInput->scroll > col) {
        cmdInput->scroll = col;
    } else if (cmdInput->scroll + cmdInput->elem.w - 1 <= col) {
        cmdInput->scroll = col - cmdInput->elem.w + 2;
    }
}

static bool _uiCmdInputKeyHandler(UICmdInput *cmdInput, int32_t key) {
    if (_uiHandleEditMode(&cmdInput->ctx, key)) {
        return true;
    }
    switch (key) {
    case TermKey_CtrlC:
    case TermKey_CtrlQ:
    case TermKey_Escape:
        cmdInput->state = UICmdInput_Canceled;
        return true;
    case '\r':
    case '\n':
        cmdInput->state = UICmdInput_Confirmed;
        return true;
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
        for (uint16_t i = 0; i < panel->elem.h / 2; i++) {
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
        for (uint16_t i = 0; i < panel->elem.h / 2; i++) {
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

static bool _uiHandleArrowKeys(Ctx *ctx, int32_t key) {
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

static bool _uiHandleEditMode(Ctx *ctx, int32_t key) {
    if (_uiHandleArrowKeys(ctx, key)) {
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
        ctxSelBegin(ctx);
        ctxCurMoveToPrevWordStart(ctx);
        ctxSelEnd(ctx);
        if (ctxSelHas(ctx)) {
            ctxRemoveBack(ctx);
        }
        break;
    case TermKey_CtrlR:
        ctxSelBegin(ctx);
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
        ctxSelBegin(ctx);
        ctxCurMoveToLineStart(ctx);
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
    case TermKey_CtrlC:
    case TermKey_CtrlQ:
    case TermKey_Escape:
        return false;
    case '\r':
        key = '\n';
        // fallthrough
    default:
        if (key == '\n' && !ctx->multiline) {
            return false;
        }
        ctxInsertCP(ctx, (UcdCP)key);
        break;
    }
    return 1;
}

static bool _uiBufHandleNormalMode(UIBufPanel *panel, int32_t key) {
    Buf *buf = bufRef(&g_ed.buffers, panel->bufHd);
    if (buf == NULL) {
        return false;
    }

    Ctx *ctx = &buf->ctx;
    if (_uiHandleNormalMovement(panel, key) || _uiHandleArrowKeys(ctx, key)) {
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
    case 'd':
        ctxIndent(ctx);
        break;
    case 'D':
        ctxDedent(ctx);
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
    if (!_uiHandleEditMode(ctx, key)) {
        panel->mode = UIBufMode_Normal;
    }
    return true;
}

static bool _uiBufHandleSelectionMode(UIBufPanel *panel, int32_t key) {
    Buf *buf = bufRef(&g_ed.buffers, panel->bufHd);
    if (buf == NULL) {
        return false;
    }

    Ctx *ctx = &buf->ctx;
    if (_uiHandleNormalMovement(panel, key) || _uiHandleArrowKeys(ctx, key)) {
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
    case 'd':
        ctxIndent(ctx);
        break;
    case 'D':
        ctxDedent(ctx);
        break;
    case TermKey_CtrlC:
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
