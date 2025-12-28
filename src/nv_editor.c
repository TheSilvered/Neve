#include <stdarg.h>
#include <assert.h>
#include <math.h>

#include "nv_buffer.h"
#include "nv_draw.h"
#include "nv_editor.h"
#include "nv_escapes.h"
#include "nv_logging.h"
#include "nv_screen.h"
#include "nv_term.h"
#include "nv_time.h"
#include "nv_tui.h"

Editor g_ed = { 0 };

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
    uiHandleKey(&g_ed.ui.elem, key);
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
