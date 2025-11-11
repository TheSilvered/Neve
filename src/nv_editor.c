#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include "nv_editor.h"
#include "nv_escapes.h"
#include "nv_screen.h"
#include "nv_term.h"
#include "nv_utils.h"

Editor g_ed = { 0 };

void editorInit(void) {
    screenInit(&g_ed.screen);
    g_ed.running = true;
    bufInit(&g_ed.fileBuf);
    ctxInit(&g_ed.saveDialogCtx, false);

    strInitFromC(&g_ed.strings.savePrompt, "File path: ");
    strInitFromC(&g_ed.strings.noFilePath, "<New File>");

    termWrite(sLen(
        escEnableAltBuffer
        escCursorShapeStillBar
    ));
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
    ctxSetFrameSize(&editorGetActiveBuf()->ctx, cols, rows - 2);

    if (g_ed.savingFile) {
        // TODO: use visual width of savePrompt
        uint16_t saveDialogWidth = cols - g_ed.strings.savePrompt.len;
        ctxSetFrameSize(&g_ed.saveDialogCtx, saveDialogWidth, 1);
        g_ed.saveDialogCtx.frame.termX = g_ed.strings.savePrompt.len;
        g_ed.saveDialogCtx.frame.termY = rows - 1;
    }

    return true;
}

static void enterFileSaveMode(void) {
    if (g_ed.savingFile) {
        return;
    }
    Ctx *ctx = editorGetActiveCtx();
    ctx->mode = CtxMode_Normal;
    g_ed.savingFile = true;
    g_ed.saveDialogCtx.mode = CtxMode_Edit;
}

static void exitFileSaveMode(void) {
    if (!g_ed.savingFile) {
        return;
    }
    g_ed.saveDialogCtx.mode = CtxMode_Normal;
    g_ed.savingFile = false;
}

static void handleKeyNormalMode(int32_t key) {
    Ctx *ctx = editorGetActiveCtx();
    switch (key) {
    case TermKey_Escape:
        exitFileSaveMode();
        return;
    case TermKey_CtrlC:
        if (g_ed.savingFile) {
            exitFileSaveMode();
        } else {
            g_ed.running = false;
        }
        return;
    case 'i':
        ctxMoveCurY(ctx, -1);
        return;
    case 'I':
        ctxCurMoveToParagraphStart(ctx);
        return;
    case TermKey_CtrlI:
        ctxCurToFileStart(ctx);
        return;
    case 'k':
        ctxMoveCurY(ctx, 1);
        return;
    case 'K':
        ctxCurMoveParagraphF(ctx);
        return;
    case TermKey_CtrlK:
        ctxCurMoveToTextEnd(ctx);
        return;
    case 'j':
        ctxMoveCurX(ctx, -1);
        return;
    case 'J':
        ctxCurMoveToWordStartB(ctx);
        return;
    case TermKey_CtrlJ:
        ctxCurMoveToWordEndB(ctx);
        return;
    case 'l':
        ctxMoveCurX(ctx, 1);
        return;
    case 'L':
        ctxCurMoveToWordEndF(ctx);
        return;
    case TermKey_CtrlL:
        ctxCurMoveToWordStartF(ctx);
        return;
    case 'u':
        ctxCurToLineStart(ctx);
        return;
    case 'o':
        ctxCurToLineEnd(ctx);
        return;
    case 'a':
        ctx->mode = CtxMode_Edit;
        return;
    case 'W':
        enterFileSaveMode();
        return;
    case 'w':
        if (editorGetActiveBuf()->path.len == 0) {
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
    case TermKey_Home:
        ctxCurToLineStart(ctx);
        return;
    case TermKey_End:
        ctxCurToLineEnd(ctx);
        return;
    case TermKey_Enter:
        if (g_ed.savingFile) {
            StrView *path = ctxGetContent(ctx);
            if (path->len != 0) {
                bufSetPath(editorGetActiveBuf(), path);
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
        case CtxMode_Edit:
            handleKeyInsertMode(key);
            return;
        }
    }
    assert(false);
}

static void renderCtxLine_(
    const Ctx *ctx,
    size_t lineIdx,
    Str *outBuf,
    uint16_t lineX,
    uint16_t lineY
) {
    size_t maxWidth = ctx->frame.w;
    size_t scrollX = ctx->frame.x;
    if (maxWidth == 0) {
        return;
    }

    strClear(outBuf, maxWidth);

    ptrdiff_t width = -scrollX;

    const char *tabFmt = "\xc2\xbb%*s"; // »%*s
    const char *startCutoffFmt = "<%*s";
    const char *endCutoffFmt = "%*s>";

    UcdCP cp = -1;
    for (
        ptrdiff_t i = ctxLineIterNextStart(ctx, lineIdx, &cp);
        i != -1;
        i = ctxLineIterNext(ctx, i, &cp)
    ) {
        uint8_t chWidth = ucdCPWidth(cp, ctx->tabStop, width + scrollX);
        width += chWidth;

        if (width <= 0) {
            continue;
        } else if (width - chWidth < 0) {
            // Draw a gray '<' at the start if a character is cut off
            strAppendFmt(outBuf, startCutoffFmt, width - 1, "");
            screenSetFg(
                &g_ed.screen,
                (ScreenColor){ .col = screenColT16(61) },
                lineX, lineY, 1
            );
        } else if ((size_t)width > maxWidth) {
            // Draw a gray '>' at the end if a character is cut off
            // If the character is a tab it can just draw a '»'
            strAppendFmt(
                outBuf,
                cp == '\t' ? tabFmt : endCutoffFmt,
                maxWidth + chWidth - width - 1, ""
            );
            screenSetFg(
                &g_ed.screen,
                (ScreenColor) { .col = screenColT16(61) },
                lineX + width - chWidth, lineY, 1
            );
            break;
        } else if (cp == '\t') {
            strAppendFmt(outBuf, tabFmt, chWidth - 1, "");
            screenSetFg(
                &g_ed.screen,
                (ScreenColor) { .col = screenColT16(61) },
                lineX + width - chWidth, lineY, 1
            );
        } else {
            UcdCh8 buf[4];
            size_t len = ucdCh8FromCP(cp, buf);
            StrView sv = { buf, len };
            strAppend(outBuf, &sv);
        }

        if ((size_t)width == maxWidth) {
            break;
        }
    }

    screenWrite(&g_ed.screen, lineX, lineY, outBuf->buf, outBuf->len);
}

static void renderFile_(Ctx *ctx) {
    Str lineBuf = { 0 };
    size_t lineCount = ctxLineCount(ctx);
    uint8_t linenoWidth = (uint8_t)(log10((double)lineCount) + 1);
    ctxSetFrameSize(ctx, g_ed.screen.w - linenoWidth - 2, g_ed.screen.h - 2);
    ctx->frame.termX = linenoWidth + 2;

    size_t totLines = NV_MIN(lineCount - ctx->frame.y, ctx->frame.h);
    for (size_t i = 0; i < totLines; i++) {
        screenWriteFmt(
            &g_ed.screen,
            0, i,
            "%*zi  ", linenoWidth, i + 1 + ctx->frame.y
        );
        screenSetFg(
            &g_ed.screen,
            (ScreenColor){ .col = screenColT16(7) },
            0, i, linenoWidth + 2
        );
        renderCtxLine_(
            ctx,
            i + ctx->frame.y,
            &lineBuf,
            linenoWidth + 2, i
        );
    }

    if (ctx->buf.len != 0) {
        return;
    }

    StrView msg = { sLen("Neve editor prototype") };
    screenWrite(
        &g_ed.screen,
        (g_ed.screen.w - msg.len) / 2,
        ctx->frame.h / 2,
        msg.buf, msg.len
    );
}

static void renderStatusBar_(void) {
    Ctx *ctx = editorGetActiveCtx();
    const char *mode;
    switch (ctx->mode) {
    case CtxMode_Edit:
        mode = "Insert";
        break;
    case CtxMode_Normal:
        mode = "Normal";
        break;
    default:
        assert(false);
    }

    Buf *activeBuf = editorGetActiveBuf();
    const char *filePath;
    if (activeBuf->path.len == 0) {
        filePath = strAsC(&g_ed.strings.noFilePath);
    } else {
        filePath = strAsC(&activeBuf->path);
    }

    screenWriteFmt(
        &g_ed.screen,
        0, g_ed.screen.h - 2,
        "%zi:%zi %s %s - %s",
        activeBuf->ctx.cur.y + 1,
        activeBuf->ctx.cur.x + 1,
        mode,
        activeBuf->ctx.edited ? "*" : "",
        filePath
    );

    screenSetTextFmt(
        &g_ed.screen,
        screenFmtInverse,
        0, g_ed.screen.h - 2, g_ed.screen.w
    );

    if (!g_ed.savingFile) {
        return;
    }

    screenWrite(
        &g_ed.screen,
        0, g_ed.screen.h - 1,
        g_ed.strings.savePrompt.buf,
        g_ed.strings.savePrompt.len
    );

    Str lineBuf = { 0 };
    renderCtxLine_(
        &g_ed.saveDialogCtx,
        0,
        &lineBuf,
        g_ed.strings.savePrompt.len,
        g_ed.screen.h - 1
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

    screenClear(&g_ed.screen, -1);
    renderFile_(&editorGetActiveBuf()->ctx);
    renderStatusBar_();

    return screenRefresh(&g_ed.screen);
}

Ctx *editorGetActiveCtx(void) {
    if (g_ed.savingFile) {
        return &g_ed.saveDialogCtx;
    }
    return &g_ed.fileBuf.ctx;
}

Buf *editorGetActiveBuf(void) {
    return &g_ed.fileBuf;
}

bool editorSaveFile(void) {
    return bufWriteToDisk(&g_ed.fileBuf);
}
