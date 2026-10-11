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
int32_t g_mapMove;
int32_t g_mapNormal, g_mapEdit, g_mapSel, g_mapCmd, g_mapGlob;

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

    Bind bind = { 0 };
    BindMatchResult res = { 0 };
    if (g_ed.runningCommand) {
        res = bindMatch(g_mapCmd, BindSeq(key), &bind);
    } else {
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
    }

    if (res != BindMatch_Found) {
        res = bindMatch(g_mapMove, BindSeq(key), &bind);
        if (res != BindMatch_Found) return;
    }

    if (!bind.hasSelection) {
        bindInvokeCallback(bind, BindSeq(key));
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

#define mkCtxBind(map, fn, ...)                                                \
    bindAdd(                                                                   \
        map,                                                                   \
        BindSeq(__VA_ARGS__),                                                  \
        (Bind){ .callback = BindCBPtr(_callCtxCB, (fn)) }                      \
    )

#define mkBind(map, cb, ...)                                                   \
    bindAdd(map, BindSeq(__VA_ARGS__), (Bind) { .callback = (cb) })

// Global map functions
static void _addMovementMap(void);
static void _addGenericMovementKeys(int32_t map);

static void _addNormalMode(void);
static void _addEditMode(void);
static void _addSelMode(void);
static void _addCmdMode(void);
static void _addGlobBinds(void);

static void _addKeyBinds(void) {
    _addMovementMap();
    _addNormalMode();
    _addEditMode();
    _addSelMode();
    _addCmdMode();
    _addGlobBinds();
}

// General bind callbacks
static void _callCtxCB(void *func);
static void _callPanelCB(void *user);
static void _normalModeCB(BindKeys keys);
static void _editModeCB(BindKeys keys);
static void _insertTextCB(BindKeys keys);
static void _goToNormalModeCB(void);
static void _selModeCB(BindKeys keys);
static void _cmdSetInputCB(int user);

static void _addMovementMap(void) {
    g_mapMove = bindAddRootMap();

    mkCtxBind(g_mapMove, ctxCurMoveUp, 'i');
    mkCtxBind(g_mapMove, ctxCurMoveDown, 'k');
    mkCtxBind(g_mapMove, ctxCurMoveLeft, 'j');
    mkCtxBind(g_mapMove, ctxCurMoveRight, 'l');
    mkCtxBind(g_mapMove, ctxCurMoveToLineStart, 'u');
    mkCtxBind(g_mapMove, ctxCurMoveToLineEnd, 'o');

    mkCtxBind(g_mapMove, ctxCurMoveToPrevParagraph, 'I');
    mkCtxBind(g_mapMove, ctxCurMoveToNextParagraph, 'K');
    mkCtxBind(g_mapMove, ctxCurMoveToPrevWordStart, 'J');
    mkCtxBind(g_mapMove, ctxCurMoveToNextWordStart, 'L');
    mkCtxBind(g_mapMove, ctxCurMoveToTextStart, 'U');
    mkCtxBind(g_mapMove, ctxCurMoveToTextEnd, 'O');

    bindAdd(g_mapMove, BindSeq(TermKey_CtrlI), (Bind){
        .callback = BindCBPtr(_callPanelCB, uiBufPanelMoveHalfUp)
    });
    bindAdd(g_mapMove, BindSeq(TermKey_CtrlK), (Bind){
        .callback = BindCBPtr(_callPanelCB, uiBufPanelMoveHalfDown)
    });
    mkCtxBind(g_mapMove, ctxCurMoveToPrevWordEnd, TermKey_CtrlJ);
    mkCtxBind(g_mapMove, ctxCurMoveToNextWordEnd, TermKey_CtrlL);
    // TODO: TermKey_CtrlU
    // TODO: TermKey_CtrlO

    // TODO: [b]c
    // TODO: [B]c
    // TODO: [^B]
    // TODO: [a]c
    // TODO: [A]c
    // TODO: [^A]

    // TODO: [f]
    // TODO: [F]
    // TODO: [n]
    // TODO: [N]

    // TODO: [space][f]s
    // TODO: [space][F]s
    //
    // TODO: [space][p]

    _addGenericMovementKeys(g_mapMove);
}

static void _addGenericMovementKeys(int32_t map) {
    // TODO: Many more useful movement keys
    mkCtxBind(map, ctxCurMoveBack, TermKey_ArrowLeft);
    mkCtxBind(map, ctxCurMoveFwd, TermKey_ArrowRight);
    mkCtxBind(map, ctxCurMoveUp, TermKey_ArrowUp);
    mkCtxBind(map, ctxCurMoveDown, TermKey_ArrowDown);

    mkCtxBind(g_mapEdit, ctxCurMoveToLineStart, TermKey_CtrlA); // Home
    mkCtxBind(g_mapEdit, ctxCurMoveToLineEnd, TermKey_CtrlE); // End
}

static void _addNormalMode(void) {
    g_mapNormal = bindAddRootMap();

    mkCtxBind(g_mapNormal, ctxUndo, 'z');
    mkCtxBind(g_mapNormal, ctxRedo, 'Z');
    // TODO: [^Z]

    // TODO: [y]s
    mkCtxBind(g_mapNormal, ctxRemoveLine, 'Y');
    mkBind(g_mapNormal, BindCBKeys(_normalModeCB), TermKey_CtrlY);
    mkCtxBind(g_mapNormal, ctxRemoveBack, 'q');
    mkCtxBind(g_mapNormal, ctxRemoveFwd, 'Q');

    // TODO: [^Q][j]c
    // TODO: [^Q][i]c
    mkCtxBind(g_mapNormal, ctxIndent, ' ', 'd');
    mkCtxBind(g_mapNormal, ctxDedent, ' ', 'D');
    // TODO: [space][b]
    // TODO: [space][j]

    // TODO: [space][s]
    // TODO: [space][S]

    // TODO: [x]s
    // TODO: [X]
    // TODO: [^X]
    // TODO: [c]s
    // TODO: [C]
    // TODO: [^C]
    // TODO: [v]
    // TODO: [V]
    // TODO: [^V]c


    // TODO: [d][i]
    // TODO: [d][k]
    // TODO: [d][J]
    // TODO: [d][L]
    // TODO: [d][^J]
    // TODO: [d][^K]
    // TODO: [d][u]
    // TODO: [d][o]
    // TODO: [d][b]c
    // TODO: [d][B]c
    // TODO: [d][a]c
    // TODO: [d][A]c

    // TODO: [d][f]
    // TODO: [d][F]
    // TODO: [d][n]
    // TODO: [d][N]
    // TODO: [d][space][f]s
    // TODO: [d][space][F]s
    // TODO: [d][space][p]

    // TODO: [D]
    // TODO: [^D][u]
    // TODO: [^D][o]
    // TODO: [^D][^D]

    // TODO: [m]c
    // TODO: [M]c
    // TODO: [space][m]

    // TODO: [w][j]
    // TODO: [w][l]
    // TODO: [w][i]
    // TODO: [w][k]

    mkBind(g_mapNormal, BindCBKeys(_normalModeCB), 'e');
    mkBind(g_mapNormal, BindCBKeys(_normalModeCB), 'E');
    mkBind(g_mapNormal, BindCBKeys(_normalModeCB), TermKey_CtrlE);
    mkBind(g_mapNormal, BindCBKeys(_normalModeCB), 'h');
    mkBind(g_mapNormal, BindCBKeys(_normalModeCB), 'H');

    // TODO: [r]s
    mkBind(g_mapNormal, BindCBKeys(_normalModeCB), 'R');
    mkBind(g_mapNormal, BindCBKeys(_normalModeCB), TermKey_CtrlR);

    mkBind(g_mapNormal, BindCBKeys(_normalModeCB), 's'); // TODO: with imm. sel
    mkBind(g_mapNormal, BindCBKeys(_normalModeCB), 'S');

    // TODO: [^S]
    // TODO: [^D]
    mkBind(g_mapNormal, BindCBKeys(_normalModeCB), 'p');
}

static void _addEditMode(void) {
    g_mapEdit = bindAddRootMap();
    _addGenericMovementKeys(g_mapEdit);

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
    mkBind(g_mapEdit, BindCBKeys(_editModeCB), TermKey_CtrlW);
    mkBind(g_mapEdit, BindCBKeys(_editModeCB), TermKey_CtrlR);
    mkBind(g_mapEdit, BindCBKeys(_editModeCB), TermKey_CtrlT);
    mkBind(g_mapEdit, BindCBKeys(_editModeCB), TermKey_CtrlY);

    mkCtxBind(g_mapEdit, ctxInsertLineAbove, TermKey_CtrlO);
    mkCtxBind(g_mapEdit, ctxInsertLineBelow, TermKey_CtrlU);
    mkCtxBind(g_mapEdit, ctxIndent, TermKey_CtrlD);
    mkCtxBind(g_mapEdit, ctxDedent, TermKey_CtrlS);

    // TODO: [^H]?
    // TODO: [^V]
    mkBind(g_mapEdit, BindCBKeys(_insertTextCB), BindAny);

    mkBind(g_mapEdit, BindCBNoArgs(_goToNormalModeCB), TermKey_CtrlC);
    mkBind(g_mapEdit, BindCBNoArgs(_goToNormalModeCB), TermKey_CtrlQ);
    mkBind(g_mapEdit, BindCBNoArgs(_goToNormalModeCB), TermKey_Escape);
}

static void _addSelMode(void) {
    g_mapSel = bindAddRootMap();

    mkBind(g_mapSel, BindCBKeys(_selModeCB), 'h');
    mkBind(g_mapSel, BindCBKeys(_selModeCB), 'H');

    // TODO: [c]
    // TODO: [x]
    // TODO: [v]
    mkBind(g_mapSel, BindCBKeys(_selModeCB), 'y');
    mkBind(g_mapSel, BindCBKeys(_selModeCB), 'r');
    mkCtxBind(g_mapSel, ctxIndent, 'd');
    mkCtxBind(g_mapSel, ctxDedent, 'D');

    // TODO: [space][j]
    // TODO: [s]c
    // TODO: [s][y]c

    mkBind(g_mapSel, BindCBNoArgs(_goToNormalModeCB), TermKey_CtrlC);
    mkBind(g_mapSel, BindCBNoArgs(_goToNormalModeCB), TermKey_CtrlQ);
    mkBind(g_mapSel, BindCBNoArgs(_goToNormalModeCB), TermKey_Escape);
}

static void _addCmdMode(void) {
    g_mapCmd = bindAddRootMap();

    Bind cancel = {
        .callback = BindCBInt(_cmdSetInputCB, UICmdInput_Canceled)
    };
    bindAdd(g_mapCmd, BindSeq(TermKey_CtrlC), cancel);
    bindAdd(g_mapCmd, BindSeq(TermKey_CtrlQ), cancel);
    bindAdd(g_mapCmd, BindSeq(TermKey_Escape), cancel);

    Bind confirm = {
        .callback = BindCBInt(_cmdSetInputCB, UICmdInput_Confirmed)
    };
    bindAdd(g_mapCmd, BindSeq('\r'), confirm);
    bindAdd(g_mapCmd, BindSeq('\n'), confirm);
}

static void _addGlobBinds(void) {
    g_mapGlob = bindAddRootMap();

    bindAdd(g_mapGlob, BindSeq('p'), (Bind){
        .callback = BindCBNoArgs(editorOpenCommandPalette)
    });
}

static void _callCtxCB(void *user) {
    Ctx *ctx = editorActiveContext();
    if (ctx != NULL) {
        ((void (*)(Ctx *))user)(ctx);
    }
}

static void _callPanelCB(void *user) {
    UIBufPanel *panel = editorActivePanel();
    if (panel != NULL) {
        ((void (*)(UIBufPanel *))user)(panel);
    }
}

static void _normalModeCB(BindKeys keys) {
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
    case TermKey_CtrlE:
        ctxCurMoveToLineStart(ctx);
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

static void _editModeCB(BindKeys keys) {
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

static void _insertTextCB(BindKeys keys) {
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

static void _goToNormalModeCB(void) {
    UIBufPanel *panel = editorActivePanel();
    Ctx *ctx = editorActiveContext();
    if (panel != NULL) {
        ctxSelCancel(ctx);
        panel->mode = UIBufMode_Normal;
    }
}

static void _selModeCB(BindKeys keys) {
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

static void _cmdSetInputCB(int user) {
    g_ed.ui.cmdInput.state = user;
}
