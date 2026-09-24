#include <assert.h>
#include <stdarg.h>

#include "nv_buffer.h"
#include "nv_context.h"
#include "nv_draw.h"
#include "nv_editor.h"
#include "nv_escapes.h"
#include "nv_key_binds.h"
#include "nv_screen.h"
#include "nv_term.h"
#include "nv_time.h"
#include "nv_tui.h"

Editor g_ed = { 0 };
int32_t g_mapNormal, g_mapEdit, g_mapSel, g_mapCmd;

static void _addKeyBinds(void);

void editorInit(void) {
    screenInit(&g_ed.screen);
    g_ed.lastUpdate = 0;

    bufMapInit(&g_ed.buffers);
    uiInit(&g_ed.ui);

    g_ed.running = true;
    g_ed.runningCommand = false;

    arrAppend(&g_ed.commands, cmdEntryNew("q", cmdQuit));
    arrAppend(&g_ed.commands, cmdEntryNew("fq", cmdForceQuit));
    arrAppend(&g_ed.commands, cmdEntryNew("quit", cmdQuit));
    arrAppend(&g_ed.commands, cmdEntryNew("exit", cmdQuit));
    arrAppend(&g_ed.commands, cmdEntryNew("w", cmdSave));
    arrAppend(&g_ed.commands, cmdEntryNew("wq", cmdSaveAndQuit));
    arrAppend(&g_ed.commands, cmdEntryNew("write", cmdSave));
    arrAppend(&g_ed.commands, cmdEntryNew("s", cmdSave));
    arrAppend(&g_ed.commands, cmdEntryNew("sq", cmdSaveAndQuit));
    arrAppend(&g_ed.commands, cmdEntryNew("save", cmdSave));
    arrAppend(&g_ed.commands, cmdEntryNew("saveas", cmdSaveAs));
    arrAppend(&g_ed.commands, cmdEntryNew("pwd", cmdWorkingDir));

    _addKeyBinds();

    termWrite(sLen(
        escEnableAltBuffer
        escCursorHide
    ));
}

void editorQuit(void) {
    screenDestroy(&g_ed.screen);
    bufMapDestroy(&g_ed.buffers);
}

bool editorUpdateSize(void) {
    uint16_t rows, cols;
    if (!termSize(&rows, &cols)) {
        return false;
    }

    screenResize(&g_ed.screen, cols, rows);
    uiResize(&g_ed.ui, cols, rows);

    return true;
}

bool editorTryExit(void) {
    Buf *buf = bufRef(&g_ed.buffers, g_ed.ui.bufPanel.bufHd);
    if (buf == NULL) {
        g_ed.running = false;
        return true;
    }
    if (buf->ctx.edited) {
        return false;
    }
    g_ed.running = false;
    return true;
}

void editorForceExit(void) {
    g_ed.running = false;
}

void editorHandleKey(int32_t key) {
    if (key == TermKey_None) return;
    if (key == TermKey_CtrlQ) editorForceExit();

    KeyBind bind = { 0 };
    BindMatchResult res = { 0 };
    if (g_ed.runningCommand) {
        res = bindMatch(g_mapCmd, BindSeq(key), &bind);
        if (res == BindMatch_Found) goto runBind;
        res = bindMatch(g_mapEdit, BindSeq(key), &bind);
        if (res == BindMatch_Found) goto runBind;
        return;
    }

    UIBufMode mode = editorActivePanel()->mode;
    switch (mode) {
    case UIBufMode_Normal:
        res = bindMatch(g_mapNormal, BindSeq(key), &bind);
        break;
    case UIBufMode_Edit:
        res = bindMatch(g_mapEdit, BindSeq(key), &bind);
        break;
    case UIBufMode_Selection:
        res = bindMatch(g_mapSel, BindSeq(key), &bind);
        break;
    }
    if (res != BindMatch_Found) return;

runBind:
    if (!bind.hasSelection) {
        bind.callback(bind.userData, BindSeq(key), (CtxSelection){ 0 });
    }
}

void _runCmd(void) {
    StrView cmd = ctxGetContent(&g_ed.ui.cmdInput.ctx);
    if (cmd.len == 0) {
        return;
    }
    StrView cmdName = cmd;
    StrView cmdArgs = { .buf = NULL, .len = 0 };
    for (size_t i = 0; i < cmd.len; i++) {
        if (cmd.buf[i] == ' ') {
            cmdName.len = i;
            cmdArgs = (StrView) {
                .buf = &cmd.buf[i + 1],
                .len = cmd.len - i - 1
            };
            break;
        }
    }

    for (size_t i = 0; i < g_ed.commands.len; i++) {
        CmdEntry *cmdEntry = g_ed.commands.items[i];
        if (cmdEntry->nameLen != cmdName.len) {
            continue;
        }
        if (memcmp(cmdEntry->name, cmdName.buf, cmdName.len) != 0) {
            continue;
        }
        if (g_ed.cmdResult != NULL) {
            memFree(g_ed.cmdResult);
        }
        g_ed.cmdResult = cmdEntry->cmd(cmdArgs);
        return;
    }
    if (g_ed.cmdResult != NULL) {
        memFree(g_ed.cmdResult);
    }
    g_ed.cmdResult = cmdResultFailed(
        "command '"strFmt"' not found",
        strArg(&cmdName)
    );
}

bool editorRefresh(void) {
    uint64_t time = timeRelNs();
    if (time - g_ed.lastUpdate < 16000000) {
        timeSleep(16000000 + g_ed.lastUpdate - time);
    }
    g_ed.lastUpdate = timeRelNs();

    if (g_ed.runningCommand) {
        switch (g_ed.ui.cmdInput.state) {
        case UICmdInput_Inserting:
            break;
        case UICmdInput_Confirmed:
            _runCmd();
            // fallthrough
        case UICmdInput_Canceled:
            g_ed.runningCommand = false;
            break;
        }
    }

    if (!editorUpdateSize()) {
        return false;
    }
    uiUpdate(&g_ed.ui.elem);

    screenClear(&g_ed.screen, -1);
    drawUI(&g_ed.screen, &g_ed.ui);

    return screenRefresh(&g_ed.screen);
}

bool editorOpen(const char *path) {
    File file;
    BufHandle newBuf = bufInvalidHandle;
    bool success = true;
    switch (fileOpen(&file, path, FileMode_Read)) {
    case FileIOResult_Success:
        bufClose(&g_ed.buffers, g_ed.ui.bufPanel.bufHd);
        if (
            bufInitFromFile(
                &g_ed.buffers,
                &file,
                &newBuf
            ).kind != BufResult_Success
        ) {
            success = false;
            fileClose(&file);
            break;
        }
        fileClose(&file);
        break;
    case FileIOResult_FileNotFound:
        bufClose(&g_ed.buffers, g_ed.ui.bufPanel.bufHd);
        newBuf = bufInitEmpty(&g_ed.buffers);
        bufSetPathC(&g_ed.buffers, newBuf, path);
        break;
    default:
        success = false;
    }
    g_ed.ui.bufPanel.bufHd = newBuf;
    return success;
}

void editorNewBuf(void) {
    bufClose(&g_ed.buffers, g_ed.ui.bufPanel.bufHd);
    BufHandle newBuf = bufInitEmpty(&g_ed.buffers);
    g_ed.ui.bufPanel.bufHd = newBuf;
}

void editorOpenCommandPalette(void) {
    g_ed.runningCommand = true;
    g_ed.ui.cmdInput.state = UICmdInput_Inserting;
    ctxDestroy(&g_ed.ui.cmdInput.ctx);
    ctxInit(&g_ed.ui.cmdInput.ctx, false);
    ctxCurAdd(&g_ed.ui.cmdInput.ctx, 0);
    if (g_ed.cmdResult != NULL) {
        memFree(g_ed.cmdResult);
        g_ed.cmdResult = NULL;
    }
}

UIBufPanel *editorActivePanel(void) {
    return g_ed.runningCommand ? NULL : &g_ed.ui.bufPanel;
}

BufHandle editorActiveBuffer(void) {
    if (g_ed.runningCommand) {
        return bufInvalidHandle;
    } else {
        return g_ed.ui.bufPanel.bufHd;
    }
}

Ctx *editorActiveContext(void) {
    if (g_ed.runningCommand) {
        return &g_ed.ui.cmdInput.ctx;
    } else {
        return &bufRef(&g_ed.buffers, g_ed.ui.bufPanel.bufHd)->ctx;
    }
}

// --------------------------- Default key binds ---------------------------- //

static void _callCtxCB(void *user, int32_t *keys, CtxSelection sel);
static void _callPanelCB(void *user, int32_t *keys, CtxSelection sel);
static void _normalModeCB(void *user, int32_t *keys, CtxSelection sel);
static void _editModeCB(void *user, int32_t *keys, CtxSelection sel);
static void _selModeCB(void *user, int32_t *keys, CtxSelection sel);
static void _insertText(void *user, int32_t *keys, CtxSelection sel);
static void _goToNormalMode(void *user, int32_t *keys, CtxSelection sel);

static void _cmdSetInput(void *user, BindKeys keys, CtxSelection sel);

static void _addNormalMode(void);
static void _addEditMode(void);
static void _addSelMode(void);
static void _addCmdMode(void);

static void _addKeyBinds(void) {
    _addNormalMode();
    _addEditMode();
    _addSelMode();
    _addCmdMode();
}

#define mkBind(map, cb, ...)                                                   \
    bindAdd(map, BindSeq(__VA_ARGS__), (KeyBind) { .callback = (cb) })

#define mkSelBind(map, cb, ...)                                                \
    bindAdd(                                                                   \
        map,                                                                   \
        BindSeq(__VA_ARGS__),                                                  \
        (KeyBind){ .callback = (cb), .hasSel = true }                          \
    )

#define mkCtxBind(map, fn, ...)                                                \
    bindAdd(                                                                   \
        map,                                                                   \
        BindSeq(__VA_ARGS__),                                                  \
        (KeyBind){ .callback = _callCtxCB, .userData = (void *)(fn) }          \
    )

static void addNormalMovement(int32_t map) {
    mkCtxBind(map, ctxCurMoveUp, 'i');
    mkCtxBind(map, ctxCurMoveToPrevParagraph, 'I');
    bindAdd(map, BindSeq(TermKey_CtrlI), (KeyBind){
        .callback = _callPanelCB,
        .userData = (void *)uiBufPanelMoveHalfUp
    });
    mkCtxBind(map, ctxCurMoveDown, 'k');
    mkCtxBind(map, ctxCurMoveToNextParagraph, 'K');
    bindAdd(map, BindSeq(TermKey_CtrlK), (KeyBind){
        .callback = _callPanelCB,
        .userData = (void *)uiBufPanelMoveHalfDown
    });
    mkCtxBind(map, ctxCurMoveToPrevParagraph, 'I');
    mkCtxBind(map, ctxCurMoveLeft, 'j');
    mkCtxBind(map, ctxCurMoveToPrevWordStart, 'J');
    mkCtxBind(map, ctxCurMoveToPrevWordEnd, TermKey_CtrlJ);
    mkCtxBind(map, ctxCurMoveRight, 'l');
    mkCtxBind(map, ctxCurMoveToNextWordEnd, 'L');
    mkCtxBind(map, ctxCurMoveToNextWordStart, TermKey_CtrlJ);
    mkCtxBind(map, ctxCurMoveToLineStart, 'u');
    mkCtxBind(map, ctxCurMoveToTextStart, 'U');
    // TODO: TermKey_CtrlU
    mkCtxBind(map, ctxCurMoveToLineEnd, 'o');
    // TODO: TermKey_CtrlO
    mkCtxBind(map, ctxCurMoveToTextEnd, 'O');
}

static void addArrowKeys(int32_t map) {
    mkCtxBind(map, ctxCurMoveBack, TermKey_ArrowLeft);
    mkCtxBind(map, ctxCurMoveFwd, TermKey_ArrowRight);
    mkCtxBind(map, ctxCurMoveUp, TermKey_ArrowUp);
    mkCtxBind(map, ctxCurMoveDown, TermKey_ArrowDown);
}

static void _addNormalMode(void) {
    g_mapNormal = bindAddRootMap();

    addNormalMovement(g_mapNormal);
    addArrowKeys(g_mapNormal);

    mkBind(g_mapNormal, _normalModeCB, 'e');
    mkBind(g_mapNormal, _normalModeCB, 'E');
    mkBind(g_mapNormal, _normalModeCB, 'h');
    mkBind(g_mapNormal, _normalModeCB, 'H');
    mkBind(g_mapNormal, _normalModeCB, 's');
    mkBind(g_mapNormal, _normalModeCB, 'S');
    mkBind(g_mapNormal, _normalModeCB, 'Y');
    mkBind(g_mapNormal, _normalModeCB, TermKey_CtrlY);
    mkCtxBind(g_mapNormal, ctxRemoveBack, 'q');
    mkCtxBind(g_mapNormal, ctxRemoveFwd, 'Q');
    mkCtxBind(g_mapNormal, ctxIndent, 'd');
    mkCtxBind(g_mapNormal, ctxDedent, 'D');
    mkBind(g_mapNormal, _normalModeCB, 'R');
    mkBind(g_mapNormal, _normalModeCB, TermKey_CtrlR);
    mkCtxBind(g_mapNormal, ctxUndo, 'z');
    mkCtxBind(g_mapNormal, ctxRedo, 'Z');
    mkBind(g_mapNormal, _normalModeCB, 'p');
}

static void _addSelMode(void) {
    g_mapSel = bindAddRootMap();

    addNormalMovement(g_mapSel);
    addArrowKeys(g_mapSel);

    mkBind(g_mapSel, _selModeCB, 'y');
    mkBind(g_mapSel, _selModeCB, 'r');
    mkBind(g_mapSel, _selModeCB, 'h');
    mkBind(g_mapSel, _selModeCB, 'H');
    mkCtxBind(g_mapSel, ctxIndent, 'd');
    mkCtxBind(g_mapSel, ctxDedent, 'D');
    mkBind(g_mapSel, _goToNormalMode, TermKey_CtrlC);
    mkBind(g_mapSel, _goToNormalMode, TermKey_CtrlQ);
    mkBind(g_mapSel, _goToNormalMode, TermKey_Escape);
}

static void _addEditMode(void) {
    g_mapEdit = bindAddRootMap();

    addArrowKeys(g_mapEdit);

    mkCtxBind(g_mapEdit, ctxCurMoveToLineStart, TermKey_CtrlA);
    mkCtxBind(g_mapEdit, ctxCurMoveToLineEnd, TermKey_CtrlE);
    mkCtxBind(g_mapEdit, ctxCurMoveFwd, TermKey_CtrlF);
    mkCtxBind(g_mapEdit, ctxCurMoveBack, TermKey_CtrlB);
    mkCtxBind(g_mapEdit, ctxCurMoveToPrevWordStart, TermKey_CtrlK);
    mkCtxBind(g_mapEdit, ctxCurMoveToNextWordEnd, TermKey_CtrlL);
    mkCtxBind(g_mapEdit, ctxCurMoveUp, TermKey_CtrlP);
    mkCtxBind(g_mapEdit, ctxCurMoveDown, TermKey_CtrlN);
    mkCtxBind(g_mapEdit, ctxRemoveBack, TermKey_CtrlZ);
    mkCtxBind(g_mapEdit, ctxRemoveBack, TermKey_Backspace);
    mkCtxBind(g_mapEdit, ctxRemoveFwd, TermKey_CtrlX);
    mkCtxBind(g_mapEdit, ctxRemoveFwd, TermKey_Delete);
    mkCtxBind(g_mapEdit, ctxIndent, TermKey_CtrlD);
    mkCtxBind(g_mapEdit, ctxDedent, TermKey_CtrlS);
    mkBind(g_mapEdit, _editModeCB, TermKey_CtrlW);
    mkBind(g_mapEdit, _editModeCB, TermKey_CtrlR);
    mkCtxBind(g_mapEdit, ctxInsertLineAbove, TermKey_CtrlO);
    mkCtxBind(g_mapEdit, ctxInsertLineBelow, TermKey_CtrlU);
    mkBind(g_mapEdit, _editModeCB, TermKey_CtrlT);
    mkBind(g_mapEdit, _editModeCB, TermKey_CtrlY);
    mkBind(g_mapEdit, _goToNormalMode, TermKey_CtrlC);
    mkBind(g_mapEdit, _goToNormalMode, TermKey_CtrlQ);
    mkBind(g_mapEdit, _goToNormalMode, TermKey_Escape);
    mkBind(g_mapEdit, _insertText, BindAny);
}

static void _addCmdMode(void) {
    g_mapCmd = bindAddRootMap();

    KeyBind cancel = {
        .callback = _cmdSetInput,
        .userData = (void *)(uintptr_t)UICmdInput_Canceled
    };
    bindAdd(g_mapCmd, BindSeq(TermKey_CtrlC), cancel);
    bindAdd(g_mapCmd, BindSeq(TermKey_CtrlQ), cancel);
    bindAdd(g_mapCmd, BindSeq(TermKey_Escape), cancel);

    KeyBind confirm = {
        .callback = _cmdSetInput,
        .userData = (void *)(uintptr_t)UICmdInput_Confirmed
    };
    bindAdd(g_mapCmd, BindSeq('\r'), confirm);
    bindAdd(g_mapCmd, BindSeq('\n'), confirm);
}

static void _callCtxCB(void *user, int32_t *keys, CtxSelection sel) {
    (void)keys;
    (void)sel;
    Ctx *ctx = editorActiveContext();
    if (ctx != NULL) {
        ((void (*)(Ctx *))user)(ctx);
    }
}

static void _callPanelCB(void *user, int32_t *keys, CtxSelection sel) {
    (void)keys;
    (void)sel;
    UIBufPanel *panel = editorActivePanel();
    if (panel != NULL) {
        ((void (*)(UIBufPanel *))user)(panel);
    }
}

static void _normalModeCB(void *user, int32_t *keys, CtxSelection sel) {
    (void)user;
    (void)sel;
    UIBufPanel *panel = editorActivePanel();
    if (panel == NULL) return;

    Buf *buf = bufRef(&g_ed.buffers, panel->bufHd);
    if (buf == NULL) return;
    Ctx *ctx = &buf->ctx;

    switch (keys[0]) {
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
        ctxSelBegin(ctx, false);
        panel->mode = UIBufMode_Selection;
        break;
    case 'S':
        ctxSelBegin(ctx, true);
        panel->mode = UIBufMode_Selection;
        break;
    case 'Y':
        ctxSelBegin(ctx, true);
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
    case 'R':
        ctxSelBegin(ctx, true);
        ctxSelEnd(ctx);
        if (ctxSelHas(ctx)) {
            ctxRemoveBack(ctx);
        }
        panel->mode = UIBufMode_Edit;
        break;
    case TermKey_CtrlR:
        ctxSelBegin(ctx, false);
        ctxCurMoveToLineEnd(ctx);
        ctxSelEnd(ctx);
        if (ctxSelHas(ctx)) {
            ctxRemoveBack(ctx);
        }
        break;
    case 'p':
        editorOpenCommandPalette();
        break;
    }
}

static void _editModeCB(void *user, int32_t *keys, CtxSelection sel) {
    (void)user;
    (void)sel;
    Ctx *ctx = editorActiveContext();
    if (ctx == NULL) return;

    switch (keys[0]) {
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
    }
}

static void _insertText(void *user, int32_t *keys, CtxSelection sel) {
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

static void _goToNormalMode(void *user, int32_t *keys, CtxSelection sel) {
    (void)user;
    (void)sel;
    (void)keys;

    UIBufPanel *panel = editorActivePanel();
    Ctx *ctx = editorActiveContext();
    if (panel != NULL) {
        ctxSelCancel(ctx);
        panel->mode = UIBufMode_Normal;
    }
}

static void _selModeCB(void *user, int32_t *keys, CtxSelection sel) {
    (void)user;
    (void)sel;
    UIBufPanel *panel = editorActivePanel();
    if (panel == NULL) return;

    Buf *buf = bufRef(&g_ed.buffers, panel->bufHd);
    if (buf == NULL) return;
    Ctx *ctx = &buf->ctx;

    switch (keys[0]) {
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
            ctxSelBegin(ctx, false);
        }
        break;
    case 'H':
        if (ctxSelIsActive(ctx)) {
            ctxSelEnd(ctx);
        } else {
            ctxSelBegin(ctx, true);
        }
        break;
    }
}

static void _cmdSetInput(void *user, BindKeys keys, CtxSelection sel) {
    (void)keys;
    (void)sel;

    g_ed.ui.cmdInput.state = (uintptr_t)user;
}

