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
    g_ed.running = true;
    bufInit(&g_ed.fileBuf);
    ctxInit(&g_ed.saveDialogCtx, false);

    strInitFromC(&g_ed.strings.savePrompt, "File path: ");

    termWrite(sLen(escCursorShapeStillBar));
}

void editorQuit(void) {
    screenDestroy(&g_ed.screen);
    bufDestroy(&g_ed.fileBuf);
    ctxDestroy(&g_ed.saveDialogCtx);
}

bool editorUpdateSize(void) {
    uint16_t rows, cols;
    if (!termSize(&rows, &cols)) {
        return false;
    }

    screenResize(&g_ed.screen, cols, rows);
    ctxSetWinSize(&g_ed.fileBuf.ctx, cols, rows - 2);

    if (g_ed.changingName) {
        // TODO: use visual width of savePrompt
        uint16_t saveDialogWidth = cols - g_ed.strings.savePrompt.len;
        ctxSetWinSize(&g_ed.saveDialogCtx, saveDialogWidth, 1);
        g_ed.saveDialogCtx.win.termX = g_ed.strings.savePrompt.len;
        g_ed.saveDialogCtx.win.termY = rows - 1;
    }

    return true;
}

static void enterFileSaveMode(void) {
    if (g_ed.changingName) {
        return;
    }
    Ctx *ctx = editorGetActiveCtx();
    ctx->mode = CtxMode_Normal;
    g_ed.changingName = true;
}

static void exitFileSaveMode(void) {
    if (!g_ed.changingName) {
        return;
    }
    g_ed.saveDialogCtx.mode = CtxMode_Normal;
    g_ed.changingName = false;
}

static void handleKeyNormalMode(int32_t key) {
    Ctx *ctx = editorGetActiveCtx();
    switch (key) {
    case TermKey_Escape:
        exitFileSaveMode();
        return;
    case TermKey_CtrlC:
        if (g_ed.changingName) {
            exitFileSaveMode();
        } else {
            g_ed.running = false;
        }
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
        ctx->mode = CtxMode_Insert;
        return;
    case 'W':
        enterFileSaveMode();
        return;
    case 'w':
        if (g_ed.fileBuf.path.len == 0) {
            enterFileSaveMode();
        } else {
            editorSaveFile();
            exitFileSaveMode();
        }
        return;
    default:
        return;
    }
}

static void handleInsertionKeys(int32_t key) {
    Ctx *ctx = editorGetActiveCtx();
    switch (key) {
    case TermKey_Backspace: {
        ctxRemoveBack(ctx);
        return;
    }
    case TermKey_Delete: {
        ctxRemoveForeward(ctx);
        return;
    }
    case '\r':
        key = '\n';
        // fallthrough
    default:
        ctxInsertCP(ctx, key);
    }
}

static void handleKeyInsertMode(int32_t key) {
    Ctx *ctx = editorGetActiveCtx();
    switch (key) {
    case TermKey_CtrlC:
    case TermKey_Escape:
        ctx->mode = CtxMode_Normal;
        return;
    default:
        handleInsertionKeys(key);
    }
}

void editorHandleKey(uint32_t key) {
    if (key == 0) {
        return;
    }

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
    case TermKey_Enter:
        if (g_ed.changingName) {
            StrView path = ctxGetLine(ctx, 0);
            if (path.len != 0) {
                bufSetPath(&g_ed.fileBuf, &path);
                editorSaveFile();
                exitFileSaveMode();
            }
            return;
        }
        // Fallthrough.
    default:
        switch (ctx->mode) {
        case CtxMode_Normal:
            handleKeyNormalMode(key);
            return;
        case CtxMode_Insert:
            handleKeyInsertMode(key);
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
        } else if (width > maxWidth + scrollX) {
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

        if (width == maxWidth + scrollX) {
            break;
        }
    }
}

static void renderFile(void) {
    Str lineBuf = { 0 };
    for (uint16_t i = 0; i < g_ed.fileBuf.ctx.win.h; i++) {
        if (i + g_ed.fileBuf.ctx.win.y < ctxLineCount(&g_ed.fileBuf.ctx)) {
            StrView line = ctxGetLine(&g_ed.fileBuf.ctx, i + g_ed.fileBuf.ctx.win.y);
            renderLine_(
                &line,
                g_ed.fileBuf.ctx.win.w,
                g_ed.fileBuf.ctx.win.x,
                &lineBuf
            );
            screenClear(&g_ed.screen, i);
            screenWrite(&g_ed.screen, 0, i, lineBuf.buf, lineBuf.len);
            continue;
        }

        screenWrite(&g_ed.screen, 0, i, sLen("~"));
    }

    if (g_ed.fileBuf.ctx.text.bufLen != 0) {
        return;
    }

    StrView msg = { sLen("Neve editor prototype") };
    screenWrite(
        &g_ed.screen,
        (g_ed.screen.w - msg.len) / 2,
        g_ed.fileBuf.ctx.win.h / 2,
        msg.buf, msg.len
    );
}

static void renderStatusBar(void) {
    Ctx *ctx = editorGetActiveCtx();
    const char *mode;
    switch (ctx->mode) {
    case CtxMode_Insert:
        mode = "Insert";
        break;
    case CtxMode_Normal:
        mode = "Normal";
        break;
    default:
        assert(false);
    }

    screenWriteFmt(
        &g_ed.screen,
        0, g_ed.screen.h - 2,
        "%zi:%zi %s %s",
        ctx->cur.y + 1,
        ctx->cur.x + 1,
        mode,
        g_ed.fileBuf.ctx.edited ? "*" : ""
    );

    if (!g_ed.changingName) {
        return;
    }

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

bool editorRefresh(void) {
    if (!editorUpdateSize()) {
        return false;
    }

    renderFile();
    renderStatusBar();

    return screenRefresh(&g_ed.screen);
}

Ctx *editorGetActiveCtx(void) {
    if (g_ed.changingName) {
        return &g_ed.saveDialogCtx;
    }
    return &g_ed.fileBuf.ctx;
}

bool editorSaveFile(void) {
    return bufWriteToDisk(&g_ed.fileBuf);
}
