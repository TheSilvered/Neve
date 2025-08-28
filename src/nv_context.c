#include <assert.h>
#include <errno.h>
#include <string.h>
#include "nv_context.h"
#include "nv_editor.h"
#include "nv_mem.h"
#include "nv_udb.h"
#include "nv_utils.h"

#define TEXT_ARR_RESIZE_IMPL_(arrName)                                         \
    if (requiredLen == arrName##Len) {                                         \
        return;                                                                \
    } else if (requiredLen == 0) {                                             \
        memFree(arrName);                                                      \
        arrName = NULL;                                                        \
        arrName##Cap = 0;                                                      \
        arrName##Len = 0;                                                      \
    } else if (requiredLen < arrName##Cap / 4) {                               \
        arrName = memShrink(                                                   \
            arrName,                                                           \
            arrName##Cap / 2,                                                  \
            sizeof(*arrName)                                                   \
        );                                                                     \
        arrName##Cap /= 2;                                                     \
    } else if (requiredLen > arrName##Cap) {                                   \
        size_t newCap = requiredLen + requiredLen / 2;                         \
        arrName = memChange(                                                   \
            arrName,                                                           \
            newCap,                                                            \
            sizeof(*arrName)                                                   \
        );                                                                     \
        arrName##Cap = newCap;                                                 \
    }

void ctxInit(Ctx *ctx, bool multiline) {
    ctx->win.x = 0;
    ctx->win.y = 0;
    ctx->win.w = 0;
    ctx->win.h = 0;
    ctx->win.termX = 0;
    ctx->win.termY = 0;

    ctx->cur.x = 0;
    ctx->cur.y = 0;
    ctx->cur.idx = 0;
    ctx->cur.baseX = 0;

    ctx->text.buf = NULL;
    ctx->text.bufLen = 0;
    ctx->text.bufCap = 0;
    ctx->text.lines = NULL;
    ctx->text.linesLen = 0;
    ctx->text.linesCap = 0;
    ctx->mode = CtxMode_Normal;
    ctx->multiline = multiline;
    ctx->edited = false;
}

void ctxDestroy(Ctx *ctx) {
    memFree(ctx->text.buf);
    memFree(ctx->text.lines);

    memset(ctx, 0, sizeof(*ctx));
}

static void textResizeBuf_(CtxText *text, size_t requiredLen) {
    TEXT_ARR_RESIZE_IMPL_(text->buf)
}

static void textResizeLines_(CtxText *text, size_t requiredLen) {
    TEXT_ARR_RESIZE_IMPL_(text->lines)
}

size_t textLineCount(const CtxText *text) {
    return text->linesLen + 1;
}

ptrdiff_t textLineChIdx(const CtxText *text, size_t lineIdx) {
    if (lineIdx > text->linesLen) {
        return -1;
    }
    if (lineIdx == 0) {
        return 0;
    } else {
        return text->lines[lineIdx - 1];
    }
}

UcdCh8 *textLinePtr(const CtxText *text, size_t lineIdx) {
    ptrdiff_t idx = textLineChIdx(text, lineIdx);
    if (idx == -1) {
        return NULL;
    }

    return text->buf + idx;
}

StrView textLine(const CtxText *text, size_t lineIdx) {
    UcdCh8 *buf = textLinePtr(text, lineIdx);
    if (buf == NULL) {
        StrView empty = {
            .buf = NULL,
            .len = 0
        };
        return empty;
    }
    UcdCh8 *lineEnd = textLinePtr(text, lineIdx + 1);
    if (lineEnd == NULL) {
        lineEnd = text->buf + text->bufLen;
    } else {
        lineEnd--; // Remove newline
    }
    StrView sv = {
        .buf = buf,
        .len = lineEnd - buf
    };
    return sv;
}

size_t textLineFromBufIdx_(const CtxText *text, size_t bufIdx) {
    if (text->linesLen == 0 || text->lines[0] > bufIdx) {
        return 0;
    } else if (bufIdx == text->bufLen) {
        return text->linesLen;
    }

    size_t lo = 0;
    size_t hi = text->linesLen;

    while (lo + 1 != hi) {
        size_t idx = (hi + lo) / 2;
        size_t line = text->lines[idx];
        if (line < bufIdx) {
            lo = idx;
        } else if (line > bufIdx) {
            hi = idx;
        } else {
            return idx + 1;
        }
    }
    return lo + 1;
}

void ctxGetCurTermPos(const Ctx *ctx, uint16_t *outCol, uint16_t *outRow) {
    uint16_t x = (uint16_t)(ctx->cur.x - ctx->win.x + ctx->win.termX);
    uint16_t y = (uint16_t)(ctx->cur.y - ctx->win.y + ctx->win.termY);
    *outCol = x;
    *outRow = y;
}

void ctxUpdateWindow_(Ctx *ctx) {
    if (ctx->cur.y >= ctx->win.h + ctx->win.y) {
        ctx->win.y = ctx->cur.y - ctx->win.h + 1;
    } else if (ctx->cur.y < ctx->win.y) {
        ctx->win.y = ctx->cur.y;
    }

    if (ctx->cur.x >= ctx->win.w + ctx->win.x) {
        ctx->win.x = ctx->cur.x - ctx->win.w + 1;
    } else if (ctx->cur.x < ctx->win.x) {
        ctx->win.x = ctx->cur.x;
    }
}

void ctxSetCurIdx_(Ctx *ctx, size_t idx) {
    if (idx > ctx->text.bufLen) {
        idx = ctx->text.bufLen;
    }

    size_t lineIdx = textLineFromBufIdx_(&ctx->text, idx);
    size_t baseLineIdx = textLineChIdx(&ctx->text, lineIdx);
    StrView line = textLine(&ctx->text, lineIdx);

    line.len = idx - baseLineIdx;
    size_t width = 0;
    UcdCP cp = -1;
    for (
        ptrdiff_t i = strViewNext(&line, -1, &cp);
        i != -1;
        i = strViewNext(&line, i, &cp)
    ) {
        width += ucdCPWidth(cp, g_ed.tabStop, width);
    }

    ctx->cur.x = width;
    ctx->cur.baseX = width;
    ctx->cur.y = lineIdx;
    ctx->cur.idx = idx;

    ctxUpdateWindow_(ctx);
}

void ctxMoveCurX(Ctx *ctx, ptrdiff_t dx) {
    if (dx == 0) {
        return;
    }

    // Move the cursor by dx characters, it may be more than dx columns.

    size_t baseLineIdx = textLineChIdx(&ctx->text, ctx->cur.y);
    size_t lineIdx = ctx->cur.idx - baseLineIdx;
    StrView line = textLine(&ctx->text, ctx->cur.y);

    if (dx > 0) {
        ptrdiff_t i;
        for (
            i = strViewNext(&line, lineIdx, NULL);
            i != -1;
            i = strViewNext(&line, i, NULL)
        ) {
            if (--dx == 0) {
                break;
            }
        }
        if (i < 0) {
            lineIdx = line.len;
        } else {
            lineIdx = i;
        }
    } else {
        ptrdiff_t i;
        for (
            i = strViewPrev(&line, lineIdx, NULL);
            i != -1;
            i = strViewPrev(&line, i, NULL)
        ) {
            if (++dx == 0) {
                break;
            }
        }
        if (i < 0) {
            lineIdx = 0;
        } else {
            lineIdx = i;
        }
    }

    ctx->cur.idx = baseLineIdx + lineIdx;

    // Find the actual column of the cursor

    line.len = lineIdx; // Get the width until lineIdx
    size_t width = 0;
    UcdCP cp = -1;
    for (
        ptrdiff_t i = strViewNext(&line, -1, &cp);
        i != -1;
        i = strViewNext(&line, i, &cp)
    ) {
        width += ucdCPWidth(cp, g_ed.tabStop, width);
    }

    ctx->cur.x = width;
    ctx->cur.baseX = width;

    ctxUpdateWindow_(ctx);
}

void ctxMoveCurY(Ctx *ctx, ptrdiff_t dy) {
    if (dy == 0) {
        return;
    }

    size_t lineCount = textLineCount(&ctx->text);
    ptrdiff_t endY = (ptrdiff_t)ctx->cur.y + dy;

    if (endY < 0) {
        endY = 0;
    } else if (endY >= (ptrdiff_t)lineCount) {
        endY = lineCount - 1;
    }

    ctx->cur.y = endY;

    StrView line = textLine(&ctx->text, endY);
    size_t baseLineIdx = textLineChIdx(&ctx->text, endY);
    size_t lineIdx = line.len;

    size_t width = 0;
    UcdCP cp = -1;
    for (
        ptrdiff_t i = strViewNext(&line, -1, &cp);
        i != -1;
        i = strViewNext(&line, i, &cp)
    ) {
        uint8_t chWidth = ucdCPWidth(cp, g_ed.tabStop, width);
        if (width + chWidth > ctx->cur.baseX) {
            lineIdx = i;
            break;
        }
        width += chWidth;
    }
    ctx->cur.x = width;
    ctx->cur.idx = baseLineIdx + lineIdx;

    ctxUpdateWindow_(ctx);
}

void ctxMoveCurIdx(Ctx *ctx, ptrdiff_t diffIdx) {
    if (diffIdx == 0) {
        return;
    }

    StrView textBuf = {
        .buf = ctx->text.buf,
        .len = ctx->text.bufLen
    };

    size_t endIdx = 0;

    if (diffIdx > 0) {
        ptrdiff_t i;
        for (
            i = strViewNext(&textBuf, ctx->cur.idx, NULL);
            i != -1;
            i = strViewNext(&textBuf, i, NULL)
        ) {
            if (--diffIdx == 0) {
                break;
            }
        }
        if (i < 0) {
            endIdx = textBuf.len;
        } else {
            endIdx = i;
        }
    } else {
        ptrdiff_t i;
        for (
            i = strViewPrev(&textBuf, ctx->cur.idx, NULL);
            i != -1;
            i = strViewPrev(&textBuf, i, NULL)
        ) {
            if (++diffIdx == 0) {
                break;
            }
        }
        if (i < 0) {
            endIdx = 0;
        } else {
            endIdx = i;
        }
    }

    ctxSetCurIdx_(ctx, endIdx);
}

void ctxMoveCurLineStart(Ctx *ctx) {
    // IMPORTANT: do not read from cur.x and cur.idx (see ctxMoveCurFileEnd)
    ctx->cur.x = 0;
    ctx->cur.baseX = 0;
    ctx->cur.idx = textLineChIdx(ctx, ctx->cur.y);
}

void ctxMoveCurLineEnd(Ctx *ctx) {
    // Iterate from the cursor position until the end of the line.

    size_t totWidth = ctx->cur.x;
    StrView line = ctxGetLine(ctx, ctx->cur.y);

    // Include the character right after the cursor
    // In case totWidth == 0 the function returns -1 which is what we want
    // anyway.
    size_t i = strViewPrev(&line, totWidth, NULL);

    UcdCP cp;
    for (
        i = strViewNext(&line, i, &cp);
        i != -1;
        i = strViewNext(&line, i, &cp)
    ) {
        totWidth += ucdCPWidth(cp, g_ed.tabStop, totWidth);
    }

    if (ctx->cur.y + 1 == ctxLineCount(ctx)) {
        ctx->cur.idx = ctx->text.bufLen;
    } else {
        // Get the index before the '\n'
        ctx->cur.idx = textLineChIdx(&ctx->text, ctx->cur.y + 1) - 1;
    }
    ctx->cur.x = totWidth;
    ctx->cur.baseX = totWidth;
}

void ctxMoveCurFileStart(Ctx *ctx) {
    ctx->cur.x = 0;
    ctx->cur.baseX = 0;
    ctx->cur.y = 0;
    ctx->cur.idx = 0;
}

void ctxMoveCurFileEnd(Ctx *ctx) {
    ctx->cur.y = ctxLineCount(ctx) - 1;
    // NOTE: here 'ctx' is in a bad state, resolve it with ctxMoveCurLineStart
    // It cannot read from ctx->cur.x or ctx->cur.idx
    ctxMoveCurLineStart(ctx);
    ctxMoveCurLineEnd(ctx);
}

void ctxInsert(Ctx *ctx, const UcdCh8 *data, size_t len) {
    if (len == 0) {
        return;
    }

    ctx->edited = true;

    size_t lineCount = 0;
    size_t trueLen = len;
    bool ignoreNL = !ctx->multiline;

    for (size_t i = 0; i < len; i++) {
        if (data[i] == '\n' && !ignoreNL) {
            lineCount++;
        } else if (data[i] == '\r' || data[i] == '\n') {
            trueLen--;
        }
    }

    // Preallocate all necessary space
    textResizeBuf_(&ctx->text, ctx->text.bufLen + trueLen);
    textResizeLines_(&ctx->text, ctx->text.linesLen + lineCount);

    UcdCh8 *buf = ctx->text.buf;
    size_t idx = ctx->cur.idx;

    // Insert the data in the buf
    memmove(
        buf + idx + trueLen,
        buf + idx,
        sizeof (*buf) * (ctx->text.bufLen - idx)
    );

    size_t copyIdx = idx;
    for (size_t i = 0; i < len; i++) {
        // Normalize all line endings to `\n`
        if (data[i] == '\r' || (data[i] == '\n' && ignoreNL)) {
            continue;
        }
        buf[copyIdx++] = data[i];
    }
    ctx->text.bufLen += trueLen;

    // Update the line indices

    size_t *lines = ctx->text.lines;

    // Offset the indices after the data
    size_t lineIdx = textLineFromBufIdx_(&ctx->text, idx);
    for (size_t i = lineIdx; i < ctx->text.linesLen; i++) {
        lines[i] += len;
    }

    if (lineCount == 0) {
        ctxSetCurIdx_(ctx, ctx->cur.idx + len);
        return;
    }

    // Make space for the new lines
    memmove(
        lines + lineIdx + lineCount,
        lines + lineIdx,
        sizeof(*lines) * (ctx->text.linesLen - lineIdx)
    );
    ctx->text.linesLen += lineCount;

    // Add the new lines
    for (size_t i = 0; i < trueLen && lineCount != 0; i++) {
        if (buf[i + idx] == '\n') {
            lines[lineIdx++] = i + idx + 1;
            lineCount--;
        }
    }
    ctxSetCurIdx_(ctx, ctx->cur.idx + len);
}

static size_t cpToUTF8Filtered_(UcdCP cp, bool allowLF, UcdCh8 *outBuf) {
    if (cp < 0 || cp > UcdCPMax) {
        return 0;
    }

    UdbCPInfo info = udbGetCPInfo(cp);
    // Do not insert control characters
    if (
        (cp != '\n' || !allowLF)
        && cp != '\t'
        && info.category >= UdbCategory_C_First
        && info.category <= UdbCategory_C_Last
    ) {
        return 0;
    }

    return ucdCh8FromCP(cp, outBuf);
}

void ctxInsertCP(Ctx *ctx, UcdCP cp) {
    if (cp == 0) {
        return;
    }

    UcdCh8 buf[4];
    size_t len = cpToUTF8Filtered_(cp, ctx->multiline, buf);
    if (len == 0) {
        return;
    }

    ctxInsert(ctx, buf, len);
}

void ctxRemove_(Ctx *ctx, size_t startIdx, size_t endIdx) {
    if (startIdx >= endIdx) {
        return;
    }
    assert(endIdx <= ctx->text.bufLen);

    ctx->edited = true;

    size_t lineCount = 0;
    for (size_t i = startIdx; i < endIdx; i++) {
        if (ctx->text.buf[i] == '\n') {
            lineCount++;
        }
    }

    CtxText *text = &ctx->text;

    memmove(
        text->buf + startIdx,
        text->buf + endIdx,
        sizeof(*text->buf) * (text->bufLen - endIdx)
    );

    text->bufLen -= endIdx - startIdx;
    textResizeBuf_(text, text->bufLen);

    size_t lineIdx = textLineFromBufIdx_(text, startIdx);

    if (lineCount != 0) {
        memmove(
            text->lines + lineIdx,
            text->lines + lineIdx + lineCount,
            sizeof(*text->lines) * (text->linesLen + 1 - lineIdx - lineCount)
        );
        text->linesLen -= lineCount;
        textResizeLines_(text, text->linesLen);
    }

    for (size_t i = lineIdx; i < text->linesLen; i++) {
        text->lines[i] -= endIdx - startIdx;
    }
}

void ctxRemoveBack(Ctx *ctx) {
    if (ctx->cur.idx == 0) {
        return;
    }

    StrView content = {
        .buf = ctx->text.buf,
        .len = ctx->text.bufLen
    };
    size_t startIdx = strViewPrev(&content, ctx->cur.idx, NULL);
    size_t endIdx = ctx->cur.idx;
    // Edit content only _after_ the cursor otherwise it could end up in
    // the middle of a multibyte sequence.
    ctxMoveCurIdx(ctx, -1);
    ctxRemove_(ctx, startIdx, endIdx);
}

void ctxRemoveForeward(Ctx *ctx) {
    if (ctx->cur.idx == ctx->text.bufLen) {
        return;
    }

    StrView content = {
        .buf = ctx->text.buf,
        .len = ctx->text.bufLen
    };
    size_t startIdx = ctx->cur.idx;
    ptrdiff_t endIdx = strViewNext(&content, ctx->cur.idx, NULL);
    if (endIdx < 0) {
        endIdx = ctx->text.bufLen;
    }
    ctxRemove_(ctx, startIdx, (size_t)endIdx);
}

void ctxSetWinSize(Ctx *ctx, uint16_t width, uint16_t height) {
    ctx->win.w = width;
    ctx->win.h = height;
    ctxUpdateWindow_(ctx);
}

size_t ctxLineCount(const Ctx *ctx) {
    return textLineCount(&ctx->text);
}

StrView ctxGetLine(const Ctx *ctx, size_t lineIdx) {
    return textLine(&ctx->text, lineIdx);
}
