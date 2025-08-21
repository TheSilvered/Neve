#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

#include "nv_editor.h"
#include "nv_escapes.h"
#include "nv_file.h"
#include "nv_mem.h"
#include "nv_term.h"
#include "nv_utils.h"

Editor g_ed = { 0 };

void editorInit(void) {
    screenInit(&g_ed.screen);
    g_ed.tabStop = 8;
    g_ed.mode = EditorMode_Normal;
    g_ed.running = true;
    ctxInitNewFile(&g_ed.fileCtx, NULL);
    ctxInitLine(&g_ed.saveDialogCtx);

    strInitFromC(&g_ed.strings.savePrompt, "File path: ");

    termWrite(sLen(escCursorShapeStillBar));
}

void editorQuit(void) {
    screenDestroy(&g_ed.screen);
    ctxDestroy(&g_ed.fileCtx);
    ctxDestroy(&g_ed.saveDialogCtx);
}

bool editorUpdateSize(void) {
    uint16_t rows, cols;
    if (!termSize(&rows, &cols)) {
        return false;
    }

    screenResize(&g_ed.screen, cols, rows);
    ctxSetWinSize(&g_ed.fileCtx, cols, rows - 1);

    if (g_ed.mode == EditorMode_SaveDialog) {
        // TODO: use visual width of savePrompt
        uint16_t saveDialogWidth = cols - g_ed.strings.savePrompt.len;
        ctxSetWinSize(&g_ed.saveDialogCtx, saveDialogWidth, 1);
        g_ed.saveDialogCtx.win.termX = g_ed.strings.savePrompt.len;
        g_ed.saveDialogCtx.win.termY = rows - 1;
    }

    return true;
}

static void handleKeyNormalMode(int32_t key) {
    Ctx *ctx = &g_ed.fileCtx;
    switch (key) {
    case TermKey_CtrlC:
        g_ed.running = false;
        return;
    case 'i':
        ctxMoveCurY(ctx, -1);
        return;
    case 'k':
        ctxMoveCurY(ctx, 1);
        return;
    case 'j':
        ctxMoveCurX(ctx, -1);
        return;
    case 'l':
        ctxMoveCurX(ctx, 1);
        return;
    case 'a':
        g_ed.mode = EditorMode_Insert;
        return;
    case 'W':
        g_ed.mode = EditorMode_SaveDialog;
        return;
    case 'w':
        if (g_ed.fileCtx.path.len == 0) {
            g_ed.mode = EditorMode_SaveDialog;
        } else {
            editorSaveFile();
        }
        return;
    default:
        return;
    }
}

static void handleKeyInsertMode(int32_t key) {
    Ctx *ctx = &g_ed.fileCtx;
    switch (key) {
    case TermKey_CtrlC:
    case TermKey_Escape:
        g_ed.mode = EditorMode_Normal;
        return;
    case TermKey_Backspace: {
        ctxRemoveBack(ctx);
        return;
    }
    case '\r':
        key = '\n';
    default:
        ctxInsertCP(ctx, key);
    }
}

static void handleKeySaveDialogMode(int32_t key) {
    Ctx *ctx = &g_ed.saveDialogCtx;
    switch (key) {
    case TermKey_Enter: {
        StrView path = ctxGetLine(ctx, 0);
        if (path.len != 0) {
            ctxSetPath(&g_ed.fileCtx, &path);
            editorSaveFile();
            g_ed.mode = EditorMode_Normal;
        }
        return;
    }
    case TermKey_Escape:
    case TermKey_CtrlC:
        g_ed.mode = EditorMode_Normal;
        return;
    case TermKey_ArrowLeft:
        ctxMoveCurX(ctx, -1);
        return;
    case TermKey_ArrowRight:
        ctxMoveCurX(ctx, 1);
        return;
    case TermKey_Backspace:
        ctxRemoveBack(ctx);
        return;
    default:
        ctxInsertCP(ctx, key);
        return;
    }
}

void editorHandleKey(uint32_t key) {
    Ctx *ctx = editorGetActiveCtx();

    switch (key) {
    case TermKey_ArrowUp:
        ctxMoveCurY(ctx, -1);
        return;
    case TermKey_ArrowDown:
        ctxMoveCurY(ctx, 1);
        return;
    case TermKey_ArrowLeft:
        ctxMoveCurX(ctx, -1);
        return;
    case TermKey_ArrowRight:
        ctxMoveCurX(ctx, 1);
        return;
    default:
        switch (g_ed.mode) {
        case EditorMode_Normal:
            handleKeyNormalMode(key);
            return;
        case EditorMode_Insert:
            handleKeyInsertMode(key);
            return;
        case EditorMode_SaveDialog:
            handleKeySaveDialogMode(key);
            return;
        }
    }
    assert(false);
}

static void renderLine_(
    const StrView *line,
    size_t maxWidth,
    size_t scrollX,
    Str *outBuf
) {
    if (maxWidth == 0) {
        return;
    }

    strClear(outBuf, maxWidth);
    char fmtBuf[32];
    StrView fmtView = { .buf = (UcdCh8 *)fmtBuf, .len = 0 };

    size_t width = 0;

    const char *tabFmt =
        escSetStyle(colorBrightBlackFg)
        "\xc2\xbb%*s" // »%*s
        escSetStyle(styleDefault);
    const char *startCutoffFmt =
        escSetStyle(colorBrightBlackFg)
        "<%*s"
        escSetStyle(styleDefault);
    const char *endCutoffFmt =
        escSetStyle(colorBrightBlackFg)
        "%*s>"
        escSetStyle(styleDefault);

    UcdCP cp = -1;
    for (
        ptrdiff_t i = strViewNext(line, -1, &cp);
        i != -1;
        i = strViewNext(line, i, &cp)
    ) {
        uint8_t chWidth = ucdCPWidth(cp, g_ed.tabStop, width);
        width += chWidth;

        if (width <= scrollX) {
            continue;
        } else if (width - chWidth < scrollX) {
            // Draw a gray '<' at the start if a character is cut off
            fmtView.len = snprintf(
                fmtBuf, NV_ARRLEN(fmtBuf),
                startCutoffFmt,
                width - scrollX - 1, ""
            );
            strAppend(outBuf, &fmtView);
        } else if (width > maxWidth) {
            // Draw a gray '>' at the end if a character is cut off
            // If the character is a tab it can just draw a '»'
            fmtView.len = snprintf(
                fmtBuf, NV_ARRLEN(fmtBuf),
                cp == '\t' ? tabFmt : endCutoffFmt,
                maxWidth + chWidth - width - 1, ""
            );
            strAppend(outBuf, &fmtView);
            break;
        } else if (cp == '\t') {
            fmtView.len = snprintf(
                fmtBuf, NV_ARRLEN(fmtBuf),
                tabFmt,
                chWidth - 1, ""
            );
            strAppend(outBuf, &fmtView);
        } else {
            StrView sv = {
                .buf = line->buf + i,
                .len = ucdCh8CPLen(cp)
            };
            strAppend(outBuf, &sv);
        }

        if (width == maxWidth) {
            break;
        }
    }
}

static void renderFile(void) {
    Str lineBuf = { 0 };
    for (uint16_t i = 0; i < g_ed.fileCtx.win.h; i++) {
        if (i + g_ed.fileCtx.win.y < ctxLineCount(&g_ed.fileCtx)) {
            StrView line = ctxGetLine(&g_ed.fileCtx, i + g_ed.fileCtx.win.y);
            renderLine_(
                &line,
                g_ed.fileCtx.win.w,
                g_ed.fileCtx.win.x,
                &lineBuf
            );
            screenClear(&g_ed.screen, i);
            screenWrite(&g_ed.screen, 0, i, lineBuf.buf, lineBuf.len);
            continue;
        }

        screenWrite(&g_ed.screen, 0, i, sLen("~"));
    }

    if (g_ed.fileCtx.text.bufLen != 0) {
        return;
    }

    StrView msg = { sLen("Neve editor prototype") };
    screenWrite(
        &g_ed.screen,
        (g_ed.screen.w - msg.len) / 2,
        g_ed.fileCtx.win.h / 2,
        msg.buf, msg.len
    );
}

static void renderSaveDialog_(void) {
    screenWrite(
        &g_ed.screen,
        0, g_ed.screen.h - 1,
        g_ed.strings.savePrompt.buf,
        g_ed.strings.savePrompt.len
    );

    StrView path = ctxGetLine(&g_ed.saveDialogCtx, 0);
    Str lineBuf = { 0 };
    renderLine_(
        &path,
        g_ed.saveDialogCtx.win.w,
        g_ed.saveDialogCtx.win.x,
        &lineBuf
    );
    screenWrite(
        &g_ed.screen,
        g_ed.strings.savePrompt.len,
        g_ed.screen.h - 1,
        lineBuf.buf,
        lineBuf.len
    );
}

static void renderStatusBar(void) {
    const char *mode;
    switch (g_ed.mode) {
    case EditorMode_Insert:
        mode = "Insert";
        break;
    case EditorMode_Normal:
        mode = "Normal";
        break;
    case EditorMode_SaveDialog: {
        renderSaveDialog_();
        return;
    }
    default:
        assert(false);
    }

    Ctx *ctx = editorGetActiveCtx();

    screenWriteFmt(
        &g_ed.screen,
        0, g_ed.screen.h - 1,
        "%zi:%zi %s %s",
        ctx->cur.y + 1,
        ctx->cur.x + 1,
        mode,
        g_ed.fileCtx.edited ? "*" : ""
    );
}

bool editorRefresh(void) {
    if (!editorUpdateSize()) {
        return false;
    }

    renderFile();
    renderStatusBar();

    return screenRefresh(&g_ed.screen);
}

Ctx *editorGetActiveCtx(void) {
    if (g_ed.mode == EditorMode_SaveDialog) {
        return &g_ed.saveDialogCtx;
    }
    return &g_ed.fileCtx;
}

bool editorSaveFile(void) {
    if (g_ed.fileCtx.path.len == 0) {
        return false;
    }
    File file;
    FileIOResult result = fileOpen(
        &file,
        strAsC(&g_ed.fileCtx.path),
        FileMode_Write
    );
    if (result != FileIOResult_Success) {
        return false;
    }
    if (!ctxWriteToFile(&g_ed.fileCtx, &file)) {
        fileClose(&file);
        return false;
    }
    fileClose(&file);
    return true;
}
