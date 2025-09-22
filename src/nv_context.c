#include <assert.h>
#include <errno.h>
#include <string.h>
#include "nv_array.h"
#include "nv_context.h"
#include "nv_udb.h"

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

    ctx->buf = (GBuf){ 0 };
    ctx->m_lines = (CtxLines){ 0 };
    ctx->mode = CtxMode_Normal;
    ctx->multiline = multiline;
    ctx->edited = false;
    ctx->tabStop = 8;
}

void ctxDestroy(Ctx *ctx) {
    memFree(ctx->buf.bytes);
    arrDestroy(&ctx->m_lines);

    memset(ctx, 0, sizeof(*ctx));
}

static size_t ctxLineChIdx_(const Ctx *ctx, size_t lineIdx) {
    assert(lineIdx <= ctx->m_lines.len);
    if (lineIdx == 0) {
        return 0;
    } else {
        return ctx->m_lines.items[lineIdx - 1];
    }
}

static size_t ctxLineLastChIdx_(const Ctx *ctx, size_t lineIdx) {
    assert(lineIdx <= ctx->m_lines.len);
    if (lineIdx == ctx->m_lines.len) {
        return ctx->buf.len;
    } else {
        return ctxLineChIdx_(ctx, lineIdx + 1) - 1; // Do not consider the \n
    }
}

size_t ctxLineLen(const Ctx *ctx, size_t lineIdx) {
    assert(lineIdx <= ctx->m_lines.len);
    return ctxLineLastChIdx_(ctx, lineIdx) - ctxLineChIdx_(ctx, lineIdx);
}

UcdCP ctxGetChF(const Ctx *ctx) {
    if (ctx->cur.idx == ctx->buf.len) {
        return -1;
    }
    return ucdCh8ToCP(gBufGetPtr(&ctx->buf, ctx->cur.idx));
}

UcdCP ctxGetChB(const Ctx *ctx) {
    if (ctx->cur.idx == 0) {
        return -1;
    }
    UcdCP cp;
    (void)ctxIterPrev(ctx, ctx->cur.idx, &cp);
    return cp;
}

static size_t ctxLineFromBufIdx_(const Ctx *ctx, size_t bufIdx) {
    if (ctx->m_lines.len == 0 || ctx->m_lines.items[0] > bufIdx) {
        return 0;
    } else if (bufIdx == ctx->buf.len) {
        return ctx->m_lines.len;
    }

    size_t lo = 0;
    size_t hi = ctx->m_lines.len;

    while (lo + 1 != hi) {
        size_t idx = (hi + lo) / 2;
        size_t line = ctx->m_lines.items[idx];
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

static void ctxUpdateWindow_(Ctx *ctx) {
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

static void ctxSetCurIdx_(Ctx *ctx, size_t idx) {
    if (idx > ctx->buf.len) {
        idx = ctx->buf.len;
    }

    size_t lineIdx = ctxLineFromBufIdx_(ctx, idx);

    size_t width = 0;
    UcdCP cp = -1;
    ptrdiff_t i;
    // The width has to be re-calculated when going backwards because there is
    // No way of knowing the width of a tab to subtract it.
    if (lineIdx != ctx->cur.y || ctx->cur.idx > idx) {
        i = ctxLineIterNextStart(ctx, lineIdx, &cp);
    } else {
        width = ctx->cur.x;
        i = ctxIterPrev(ctx, ctx->cur.idx, NULL);
        i = ctxIterNext(ctx, i, &cp);
    }

    for (; i >= 0 && (size_t)i < idx; i = ctxLineIterNext(ctx, i, &cp)) {
        width += ucdCPWidth(cp, ctx->tabStop, width);
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
        ptrdiff_t lastCh = ctxLineLastChIdx_(ctx, ctx->cur.y);
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
        ptrdiff_t firstCh = ctxLineChIdx_(ctx, ctx->cur.y);
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
        width += ucdCPWidth(cp, ctx->tabStop, width);
    }

    ctx->cur.x = width;
    ctx->cur.baseX = width;

    ctxUpdateWindow_(ctx);
}

void ctxMoveCurY(Ctx *ctx, ptrdiff_t dy) {
    if (dy == 0) {
        return;
    }

    size_t lineCount = ctxLineCount(ctx);
    ptrdiff_t endY = (ptrdiff_t)ctx->cur.y + dy;

    if (endY < 0) {
        endY = 0;
    } else if (endY >= (ptrdiff_t)lineCount) {
        endY = lineCount - 1;
    }

    ctx->cur.y = endY;

    size_t lineIdx = ctxLineLastChIdx_(ctx, endY);

    size_t width = 0;
    UcdCP cp = -1;
    for (
        ptrdiff_t i = ctxLineIterNextStart(ctx, endY, &cp);
        i != -1;
        i = ctxLineIterNext(ctx, i, &cp)
    ) {
        uint8_t chWidth = ucdCPWidth(cp, ctx->tabStop, width);
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
            endIdx = ctx->buf.len;
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
    ctx->cur.x = 0;
    ctx->cur.baseX = 0;
    ctx->cur.idx = ctxLineChIdx_(ctx, ctx->cur.y);
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
        totWidth += ucdCPWidth(cp, ctx->tabStop, totWidth);
    }

    if (ctx->cur.y + 1 == ctxLineCount(ctx)) {
        ctx->cur.idx = ctx->buf.len;
    } else {
        // Get the index before the '\n'
        ctx->cur.idx = ctxLineChIdx_(ctx, ctx->cur.y + 1) - 1;
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
    ctxSetCurIdx_(ctx, ctx->buf.len);
}

void ctxMoveCurWordStartF(Ctx *ctx) {
    if (ctx->cur.idx == ctx->buf.len) {
        return;
    }
    ptrdiff_t i = ctx->cur.idx;
    UcdCP cp = ctxGetChF(ctx);

    if (ucdIsCPAlphanumeric(cp)) {
        for (
            i = ctxIterNext(ctx, i, &cp);
            i != -1 && ucdIsCPAlphanumeric(cp);
            i = ctxIterNext(ctx, i, &cp)
        ) { }
    } else {
        for (
            i = ctxIterNext(ctx, i, &cp);
            i != -1 && !ucdIsCPWhiteSpace(cp) && !ucdIsCPAlphanumeric(cp);
            i = ctxIterNext(ctx, i, &cp)
        ) { }
    }

    // Skip white space
    for (; i != -1 && ucdIsCPWhiteSpace(cp); i = ctxIterNext(ctx, i, &cp)) { }

    if (i == -1) {
        i = ctx->buf.len;
    }
    ctxSetCurIdx_(ctx, i);
}

void ctxMoveCurWordEndF(Ctx *ctx) {
    if (ctx->cur.idx == ctx->buf.len) {
        return;
    }
    ptrdiff_t i = ctx->cur.idx;
    UcdCP cp = ctxGetChF(ctx);

    // Skip white space
    for (; i != -1 && ucdIsCPWhiteSpace(cp); i = ctxIterNext(ctx, i, &cp)) { }

    if (ucdIsCPAlphanumeric(cp) && i != -1) {
        for (
            i = ctxIterNext(ctx, i, &cp);
            i != -1 && ucdIsCPAlphanumeric(cp);
            i = ctxIterNext(ctx, i, &cp)
        ) { }
    } else if (i != -1) {
        for (
            i = ctxIterNext(ctx, i, &cp);
            i != -1 && !ucdIsCPWhiteSpace(cp) && !ucdIsCPAlphanumeric(cp);
            i = ctxIterNext(ctx, i, &cp)
        ) { }
    }
    if (i == -1) {
        i = ctx->buf.len;
    }
    ctxSetCurIdx_(ctx, i);
}

void ctxMoveCurWordStartB(Ctx *ctx) {
    if (ctx->cur.idx == 0) {
        return;
    }
    ptrdiff_t i = ctx->cur.idx;
    UcdCP cp = ctxGetChB(ctx);

    // Skip white space
    for (; i != -1 && ucdIsCPWhiteSpace(cp); i = ctxIterPrev(ctx, i, &cp)) { }

    if (ucdIsCPAlphanumeric(cp) && i != -1) {
        for (
            i = ctxIterPrev(ctx, i, &cp);
            i != -1 && ucdIsCPAlphanumeric(cp);
            i = ctxIterPrev(ctx, i, &cp)
        ) { }
    } else if (i != -1) {
        for (
            i = ctxIterPrev(ctx, i, &cp);
            i != -1 && !ucdIsCPWhiteSpace(cp) && !ucdIsCPAlphanumeric(cp);
            i = ctxIterPrev(ctx, i, &cp)
        ) { }
    }
    if (i == -1) {
        i = 0;
    } else {
        // Get the index of the character after the character found. Because the
        // cursor is displayed to the left of the character.
        i = ctxIterNext(ctx, i, NULL);
    }
    ctxSetCurIdx_(ctx, i);
}

void ctxMoveCurWordEndB(Ctx *ctx) {
    if (ctx->cur.idx == 0) {
        return;
    }
    ptrdiff_t i = ctx->cur.idx;
    UcdCP cp = ctxGetChB(ctx);

    if (ucdIsCPAlphanumeric(cp)) {
        for (
            i = ctxIterPrev(ctx, i, &cp);
            i != -1 && ucdIsCPAlphanumeric(cp);
            i = ctxIterPrev(ctx, i, &cp)
        ) { }
    } else {
        for (
            i = ctxIterPrev(ctx, i, &cp);
            i != -1 && !ucdIsCPWhiteSpace(cp) && !ucdIsCPAlphanumeric(cp);
            i = ctxIterPrev(ctx, i, &cp)
        ) { }
    }

    // Skip white space
    for (; i != -1 && ucdIsCPWhiteSpace(cp); i = ctxIterPrev(ctx, i, &cp)) { }

    if (i == -1) {
        i = 0;
    } else {
        // Get the index of the character after the character found. Because the
        // cursor is displayed to the left of the character.
        i = ctxIterNext(ctx, i, NULL);
    }
    ctxSetCurIdx_(ctx, i);
}

void ctxMoveCurParagraphF(Ctx *ctx) {
    if (ctx->cur.y + 1 >= ctxLineCount(ctx)) {
        ctxMoveCurLineEnd(ctx);
        return;
    }

    size_t i = ctx->cur.y + 1;
    for (; i < ctxLineCount(ctx) - 1 && ctxLineLen(ctx, i) != 0; i++) {}

    ctxSetCurIdx_(ctx, ctxLineLastChIdx_(ctx, i));
}

void ctxMoveCurParagraphB(Ctx *ctx) {
    if (ctx->cur.y == 0) {
        ctxMoveCurLineStart(ctx);
        return;
    }

    size_t i = ctx->cur.y - 1;
    for (; i > 0 && ctxLineLen(ctx, i) != 0; i--) {}

    ctxSetCurIdx_(ctx, ctxLineChIdx_(ctx, i));
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
    arrResize(&ctx->m_lines, ctx->m_lines.len + lineCount);

    GBuf *buf = &ctx->buf;
    size_t idx = ctx->cur.idx;

    // Insert the data in the buf
    gBufSetGapIdx(buf, ctx->cur.idx);

    size_t lineStart = 0;
    for (size_t i = 0; i < len; i++) {
        // Normalize all line endings to `\n`
        if (data[i] == '\r' || (data[i] == '\n' && ignoreNL)) {
            gBufInsert(buf, &data[lineStart], i - lineStart - 1);
            lineStart = i + 1;
        }
    }

    if (lineStart < len) {
        gBufInsert(buf, &data[lineStart], len - lineStart);
    }

    // Update the line indices

    size_t *lines = ctx->m_lines.items;

    // Offset the indices after the data
    size_t lineIdx = ctxLineFromBufIdx_(ctx, idx);
    for (size_t i = lineIdx; i < ctx->m_lines.len; i++) {
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
        sizeof(*lines) * (ctx->m_lines.len - lineIdx)
    );
    ctx->m_lines.len += lineCount;

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

void ctxAppend(Ctx *ctx, const UcdCh8 *data, size_t len) {
    if (len == 0) {
        return;
    }

    ctx->edited = true;

    size_t lineStart = 0;
    bool ignoreNL = !ctx->multiline;

    GBuf *buf = &ctx->buf;

    for (size_t i = 0; i < len; i++) {
        if (data[i] == '\n' && !ignoreNL) {
            gBufInsert(buf, &data[lineStart], i - lineStart);
            gBufInsert(buf, (const UcdCh8 *)"\n", 1);
            lineStart = i + 1;
            arrAppend(&ctx->m_lines, buf->len);
        } else if (data[i] == '\r' || data[i] == '\n') {
            gBufInsert(buf, &data[lineStart], i - lineStart);
            lineStart = i + 1;
        }
    }

    if (lineStart < len) {
        gBufInsert(buf, &data[lineStart], len - lineStart);
    }
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
        && UdbMajorCategory(info.category) == UdbCategory_C
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
    assert(endIdx <= ctx->buf.len);

    ctx->edited = true;

    gBufSetGapIdx(&ctx->buf, endIdx);

    size_t lineCount = 0;
    for (size_t i = startIdx; i < endIdx; i++) {
        if (ctx->buf.bytes[i] == '\n') {
            lineCount++;
        }
    }

    gBufRemove(&ctx->buf, endIdx - startIdx);

    size_t lineIdx = ctxLineFromBufIdx_(ctx, startIdx);
    size_t *lines = ctx->m_lines.items;

    if (lineCount != 0) {
        memmove(
            lines + lineIdx,
            lines + lineIdx + lineCount,
            sizeof(*lines) * (ctx->m_lines.len + 1 - lineIdx - lineCount)
        );
        ctx->m_lines.len -= lineCount;
        arrResize(&ctx->m_lines, ctx->m_lines.len);
    }

    for (size_t i = lineIdx; i < ctx->m_lines.len; i++) {
        ctx->m_lines.items[i] -= endIdx - startIdx;
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
    if (ctx->cur.idx == ctx->buf.len) {
        return;
    }

    size_t startIdx = ctx->cur.idx;
    ptrdiff_t endIdx = ctxIterNext(ctx, ctx->cur.idx, NULL);
    if (endIdx < 0) {
        endIdx = ctx->buf.len;
    }
    ctxRemove_(ctx, startIdx, (size_t)endIdx);
}

void ctxSetFrameSize(Ctx *ctx, uint16_t width, uint16_t height) {
    ctx->frame.w = width;
    ctx->frame.h = height;
    ctxUpdateWindow_(ctx);
}

size_t ctxLineCount(const Ctx *ctx) {
    return ctx->m_lines.len + 1;
}

StrView *ctxGetContent(Ctx *ctx) {
    gBufUnite(&ctx->buf);
    return (StrView *)&ctx->buf;
}

ptrdiff_t ctxIterNext(const Ctx *ctx, ptrdiff_t idx, UcdCP *outCP) {
    return gBufNext(&ctx->buf, idx, outCP);
}

ptrdiff_t ctxIterPrev(const Ctx *ctx, ptrdiff_t idx, UcdCP *outCP) {
    return gBufPrev(&ctx->buf, idx, outCP);
}

ptrdiff_t ctxLineIterNextStart(const Ctx *ctx, size_t lineIdx, UcdCP *outCP) {
    ptrdiff_t i = (ptrdiff_t)ctxLineChIdx_(ctx, lineIdx) - 1;
    return ctxLineIterNext(ctx, i, outCP);
}

ptrdiff_t ctxLineIterPrevStart(const Ctx *ctx, size_t lineIdx, UcdCP *outCP) {
    ptrdiff_t i = ctxLineLastChIdx_(ctx, lineIdx) + 1;
    if ((size_t)i > ctx->buf.len) {
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
