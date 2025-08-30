#include <assert.h>
#include <errno.h>
#include <string.h>
#include "nv_array.h"
#include "nv_context.h"
#include "nv_editor.h"
#include "nv_udb.h"
#include "nv_utils.h"

void ctxInit(Ctx *ctx, bool multiline) {
    ctx->frame = (CtxFrame) {
        .x = 0,
        .y = 0,
        .w = 0,
        .h = 0,
        .termX = 0,
        .termY = 0
    };

    ctx->cur = (CtxCursor) {
        .x = 0,
        .y = 0,
        .idx = 0,
        .baseX = 0
    };

    ctx->text.buf = (GBuf){ 0 };
    ctx->text.m_lines = (CtxTextLines){ 0 };
    ctx->mode = CtxMode_Normal;
    ctx->multiline = multiline;
    ctx->edited = false;
}

void ctxDestroy(Ctx *ctx) {
    memFree(ctx->text.buf.bytes);
    arrDestroy(&ctx->text.m_lines);

    memset(ctx, 0, sizeof(*ctx));
}

size_t textLineCount(const CtxText *text) {
    return text->m_lines.len + 1;
}

size_t textLineChIdx(const CtxText *text, size_t lineIdx) {
    assert(lineIdx <= text->m_lines.len);
    if (lineIdx == 0) {
        return 0;
    } else {
        return text->m_lines.items[lineIdx - 1];
    }
}

size_t textLineLastChIdx(const CtxText *text, size_t lineIdx) {
    assert(lineIdx <= text->m_lines.len);
    if (lineIdx == text->m_lines.len) {
        return text->buf.len;
    } else {
        return textLineChIdx(text, lineIdx + 1) - 1; // Do not consider the \n
    }
}

size_t textLineLen(const CtxText *text, size_t lineIdx) {
    assert(lineIdx <= text->m_lines.len);
    return textLineLastChIdx(text, lineIdx) - textLineChIdx(text, lineIdx);
}

size_t textLineFromBufIdx_(const CtxText *text, size_t bufIdx) {
    if (text->m_lines.len == 0 || text->m_lines.items[0] > bufIdx) {
        return 0;
    } else if (bufIdx == text->buf.len) {
        return text->m_lines.len;
    }

    size_t lo = 0;
    size_t hi = text->m_lines.len;

    while (lo + 1 != hi) {
        size_t idx = (hi + lo) / 2;
        size_t line = text->m_lines.items[idx];
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
    uint16_t x = (uint16_t)(ctx->cur.x - ctx->frame.x + ctx->frame.termX);
    uint16_t y = (uint16_t)(ctx->cur.y - ctx->frame.y + ctx->frame.termY);
    *outCol = x;
    *outRow = y;
}

void ctxUpdateWindow_(Ctx *ctx) {
    if (ctx->cur.y >= ctx->frame.h + ctx->frame.y) {
        ctx->frame.y = ctx->cur.y - ctx->frame.h + 1;
    } else if (ctx->cur.y < ctx->frame.y) {
        ctx->frame.y = ctx->cur.y;
    }

    if (ctx->cur.x >= ctx->frame.w + ctx->frame.x) {
        ctx->frame.x = ctx->cur.x - ctx->frame.w + 1;
    } else if (ctx->cur.x < ctx->frame.x) {
        ctx->frame.x = ctx->cur.x;
    }
}

void ctxSetCurIdx_(Ctx *ctx, size_t idx) {
    if (idx > ctx->text.buf.len) {
        idx = ctx->text.buf.len;
    }

    size_t lineIdx = textLineFromBufIdx_(&ctx->text, idx);

    size_t width = 0;
    UcdCP cp = -1;
    for (
        ptrdiff_t i = ctxLineIterNextStart(ctx, lineIdx, &cp);
        i >= 0 && (size_t)i < idx;
        i = ctxLineIterNext(ctx, i, &cp)
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

    if (dx > 0) {
        ptrdiff_t lastCh = textLineLastChIdx(&ctx->text, ctx->cur.y);
        ptrdiff_t i;
        for (
            i = ctxLineIterNext(ctx, ctx->cur.idx, NULL);
            i != -1 && i < lastCh;
            i = ctxLineIterNext(ctx, i, NULL)
        ) {
            if (--dx == 0) {
                break;
            }
        }
        if (i < 0 || i > lastCh) {
            ctx->cur.idx = lastCh;
        } else {
            ctx->cur.idx = i;
        }
    } else {
        ptrdiff_t firstCh = textLineChIdx(&ctx->text, ctx->cur.y);
        ptrdiff_t i;
        for (
            i = ctxLineIterPrev(ctx, ctx->cur.idx, NULL);
            i != -1 && i > firstCh;
            i = ctxLineIterPrev(ctx, i, NULL)
        ) {
            if (++dx == 0) {
                break;
            }
        }
        if (i < 0 || i < firstCh) {
            ctx->cur.idx = firstCh;
        } else {
            ctx->cur.idx = i;
        }
    }

    // Find the actual column of the cursor

    size_t width = 0;
    UcdCP cp = -1;
    for (
        ptrdiff_t i = ctxLineIterNextStart(ctx, ctx->cur.y, &cp);
        i >= 0 && (size_t)i < ctx->cur.idx;
        i = ctxLineIterNext(ctx, i, &cp)
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

    size_t lineIdx = textLineLastChIdx(&ctx->text, endY);

    size_t width = 0;
    UcdCP cp = -1;
    for (
        ptrdiff_t i = ctxLineIterNextStart(ctx, endY, &cp);
        i != -1;
        i = ctxLineIterNext(ctx, i, &cp)
    ) {
        uint8_t chWidth = ucdCPWidth(cp, g_ed.tabStop, width);
        if (width + chWidth > ctx->cur.baseX) {
            lineIdx = i;
            break;
        }
        width += chWidth;
    }
    ctx->cur.x = width;
    ctx->cur.idx = lineIdx;

    ctxUpdateWindow_(ctx);
}

void ctxMoveCurIdx(Ctx *ctx, ptrdiff_t diffIdx) {
    if (diffIdx == 0) {
        return;
    }

    size_t endIdx = 0;

    if (diffIdx > 0) {
        ptrdiff_t i;
        for (
            i = ctxIterNext(ctx, ctx->cur.idx, NULL);
            i != -1;
            i = ctxIterNext(ctx, i, NULL)
        ) {
            if (--diffIdx == 0) {
                break;
            }
        }
        if (i < 0) {
            endIdx = ctx->text.buf.len;
        } else {
            endIdx = i;
        }
    } else {
        ptrdiff_t i;
        for (
            i = ctxIterPrev(ctx, ctx->cur.idx, NULL);
            i != -1;
            i = ctxIterPrev(ctx, i, NULL)
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
    ctx->cur.idx = textLineChIdx(&ctx->text, ctx->cur.y);
}

void ctxMoveCurLineEnd(Ctx *ctx) {
    // Iterate from the cursor position until the end of the line.

    size_t totWidth = ctx->cur.x;

    // Include the character right after the cursor
    // In case totWidth == 0 the function returns -1 which is what we want
    // anyway.
    ptrdiff_t i = ctxIterPrev(ctx, ctx->cur.idx, NULL);

    UcdCP cp;
    for (
        i = ctxLineIterNext(ctx, i, &cp);
        i != -1;
        i = ctxLineIterNext(ctx, i, &cp)
    ) {
        totWidth += ucdCPWidth(cp, g_ed.tabStop, totWidth);
    }

    if (ctx->cur.y + 1 == ctxLineCount(ctx)) {
        ctx->cur.idx = ctx->text.buf.len;
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
    arrResize(&ctx->text.m_lines, ctx->text.m_lines.len + lineCount);

    GBuf *buf = &ctx->text.buf;
    size_t idx = ctx->cur.idx;

    // Insert the data in the buf
    gBufSetGapIdx(buf, ctx->cur.idx);

    for (size_t i = 0; i < len; i++) {
        // Normalize all line endings to `\n`
        if (data[i] == '\r' || (data[i] == '\n' && ignoreNL)) {
            continue;
        }
        gBufInsert(buf, &data[i], 1);
    }

    // Update the line indices

    size_t *lines = ctx->text.m_lines.items;

    // Offset the indices after the data
    size_t lineIdx = textLineFromBufIdx_(&ctx->text, idx);
    for (size_t i = lineIdx; i < ctx->text.m_lines.len; i++) {
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
        sizeof(*lines) * (ctx->text.m_lines.len - lineIdx)
    );
    ctx->text.m_lines.len += lineCount;

    // Add the new lines
    for (size_t i = 0; i < trueLen && lineCount != 0; i++) {
        // Safe to read directly, it is all after gapIdx
        if (buf->bytes[i + idx] == '\n') {
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
    assert(endIdx <= ctx->text.buf.len);

    ctx->edited = true;

    gBufSetGapIdx(&ctx->text.buf, endIdx);

    size_t lineCount = 0;
    for (size_t i = startIdx; i < endIdx; i++) {
        if (ctx->text.buf.bytes[i] == '\n') {
            lineCount++;
        }
    }

    CtxText *text = &ctx->text;

    gBufRemove(&ctx->text.buf, endIdx - startIdx);

    size_t lineIdx = textLineFromBufIdx_(text, startIdx);
    size_t *lines = text->m_lines.items;

    if (lineCount != 0) {
        memmove(
            lines + lineIdx,
            lines + lineIdx + lineCount,
            sizeof(*lines) * (text->m_lines.len + 1 - lineIdx - lineCount)
        );
        text->m_lines.len -= lineCount;
        arrResize(&text->m_lines, text->m_lines.len);
    }

    for (size_t i = lineIdx; i < text->m_lines.len; i++) {
        text->m_lines.items[i] -= endIdx - startIdx;
    }
}

void ctxRemoveBack(Ctx *ctx) {
    if (ctx->cur.idx == 0) {
        return;
    }

    size_t startIdx = ctxIterPrev(ctx, ctx->cur.idx, NULL);
    size_t endIdx = ctx->cur.idx;
    // Edit content only _after_ the cursor otherwise it could end up in
    // the middle of a multibyte sequence.
    ctxMoveCurIdx(ctx, -1);
    ctxRemove_(ctx, startIdx, endIdx);
}

void ctxRemoveForeward(Ctx *ctx) {
    if (ctx->cur.idx == ctx->text.buf.len) {
        return;
    }

    size_t startIdx = ctx->cur.idx;
    ptrdiff_t endIdx = ctxIterNext(ctx, ctx->cur.idx, NULL);
    if (endIdx < 0) {
        endIdx = ctx->text.buf.len;
    }
    ctxRemove_(ctx, startIdx, (size_t)endIdx);
}

void ctxSetFrameSize(Ctx *ctx, uint16_t width, uint16_t height) {
    ctx->frame.w = width;
    ctx->frame.h = height;
    ctxUpdateWindow_(ctx);
}

size_t ctxLineCount(const Ctx *ctx) {
    return textLineCount(&ctx->text);
}

StrView *ctxGetContent(Ctx *ctx) {
    gBufUnite(&ctx->text.buf);
    return (StrView *)&ctx->text.buf;
}

ptrdiff_t ctxIterNext(const Ctx *ctx, ptrdiff_t idx, UcdCP *outCP) {
    return gBufNext(&ctx->text.buf, idx, outCP);
}

ptrdiff_t ctxIterPrev(const Ctx *ctx, ptrdiff_t idx, UcdCP *outCP) {
    return gBufPrev(&ctx->text.buf, idx, outCP);
}

ptrdiff_t ctxLineIterNextStart(const Ctx *ctx, size_t lineIdx, UcdCP *outCP) {
    ptrdiff_t i = (ptrdiff_t)textLineChIdx(&ctx->text, lineIdx) - 1;
    return ctxLineIterNext(ctx, i, outCP);
}

ptrdiff_t ctxLineIterPrevStart(const Ctx *ctx, size_t lineIdx, UcdCP *outCP) {
    ptrdiff_t i = textLineLastChIdx(&ctx->text, lineIdx) + 1;
    if ((size_t)i > ctx->text.buf.len) {
        i = -1;
    }
    return ctxLineIterPrev(ctx, i, outCP);
}

ptrdiff_t ctxLineIterNext(const Ctx *ctx, ptrdiff_t idx, UcdCP *outCP) {
    UcdCP cp;
    ptrdiff_t newIdx = ctxIterNext(ctx, idx, &cp);
    if (cp == '\n') {
        cp = 0;
        newIdx = -1;
    }
    if (outCP != NULL) {
        *outCP = cp;
    }
    return newIdx;
}

ptrdiff_t ctxLineIterPrev(const Ctx *ctx, ptrdiff_t idx, UcdCP *outCP) {
    UcdCP cp;
    ptrdiff_t newIdx = ctxIterPrev(ctx, idx, &cp);
    if (cp == '\n') {
        cp = 0;
        newIdx = -1;
    }
    if (outCP != NULL) {
        *outCP = cp;
    }
    return newIdx;
}
