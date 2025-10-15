#include <string.h>
#include <assert.h>
#include "nv_context.h"
#include "nv_mem.h"
#include "nv_unicode.h"

#ifndef lineRefMaxGap_
#define lineRefMaxGap_ (2048)
#endif // !lineRefMaxGap_

// Get from the gap buffer
static inline UcdCh8 *ctxBufGet_(const CtxBuf *buf, size_t idx);
// Reserve space in the gap buffer
static void ctxBufReserve_(CtxBuf *buf, size_t amount);
// Shrkin if too much space is empty
static void ctxBufShrink_(CtxBuf *buf);
// Insert at the gapIdx and move it
static void ctxBufInsert_(CtxBuf *buf, const UcdCh8 *text, size_t len);
// Remove before the gap index and move it
static void ctxBufRemove_(CtxBuf *buf, size_t len);
// Change the gap index
static void ctxBufSetGapIdx_(CtxBuf *buf, size_t gapIdx);

// Get info about the line that `idx` is in, outY is the line number (from 0)
// outIdx is the start index of the line
static void ctxLineAt_(
    const Ctx *ctx,
    size_t idx,
    size_t *outLineNo,
    size_t *outStartIdx
);

// Get the index of the first character of a line
static ptrdiff_t ctxLineToIdx_(const Ctx *ctx, size_t lineNo);

// Get the index of the cursor at idx or  of the cursor on where it should be
// inserted
static size_t ctxCursorAt_(Ctx *ctx, size_t idx);
static void ctxCursorAdd_(Ctx *ctx, size_t idx);
static void ctxCursorRemove_(Ctx *ctx, size_t idx);
static void ctxCursorReplace_(Ctx *ctx, size_t old, size_t new);

void ctxInit(Ctx *ctx, bool multiline) {
    ctx->m_lineRefs = (CtxLineRefs){ 0 };
    ctx->m_cursors = (CtxCursors){ 0 };
    ctx->m_selects = (CtxSelects){ 0 };

    ctx->m_buf = (CtxBuf) {
        .bytes = NULL,
        .len = 0,
        .cap = 0,
        .gapIdx = 0
    };

    ctx->mode = CtxMode_Normal;
    ctx->edited = false;
    ctx->multiline = multiline;
    ctx->tabStop = 8;
}

void ctxDestroy(Ctx *ctx) {
    arrDestroy(&ctx->m_lineRefs);
    arrDestroy(&ctx->m_cursors);
    arrDestroy(&ctx->m_selects);

    memFree(ctx->m_buf.bytes);

    // Keep the context in a valid state after deletion
    ctxInit(ctx, ctx->multiline);
}

static inline UcdCh8 *ctxBufGet_(const CtxBuf *buf, size_t idx) {
    assert(idx < buf->len);
    if (idx >= buf->gapIdx) {
        size_t gapSize = buf->cap - buf->len;
        idx += gapSize;
    }
    return &buf->bytes[idx];
}

static void ctxBufReserve_(CtxBuf *buf, size_t amount) {
    size_t requiredLen = buf->len + amount;
    if (requiredLen <= buf->cap) {
        return;
    }

    size_t newCap = requiredLen + requiredLen / 2;
    buf->bytes = memChange(
        buf->bytes,
        newCap,
        sizeof(*buf->bytes)
    );

    size_t newGapSize = newCap - buf->len;
    size_t gapSize = buf->cap - buf->len;

    memmove(
        buf->bytes + buf->gapIdx + newGapSize,
        buf->bytes + buf->gapIdx + gapSize,
        (buf->len - buf->gapIdx) * sizeof(*buf->bytes)
    );
    buf->cap = newCap;
}

static void ctxBufShrink_(CtxBuf *buf) {
    if (buf->len >= buf->cap / 4) {
        return;
    }
    size_t newCap = buf->cap / 2;
    size_t newGapSize = newCap - buf->len;
    size_t gapSize = buf->cap - buf->len;
    memmove(
        buf->bytes + buf->gapIdx + newGapSize,
        buf->bytes + buf->gapIdx + gapSize,
        (buf->len - buf->gapIdx) * sizeof(*buf->bytes)
    );

    buf->bytes = memShrink(
        buf->bytes,
        buf->cap / 2,
        sizeof(*buf->bytes)
    );

    buf->cap = newCap;
}

static void ctxBufInsert_(CtxBuf *buf, const UcdCh8 *text, size_t len) {
    ctxBufReserve_(buf, len);
    memcpy(buf->bytes + buf->gapIdx, text, len * sizeof(*text));
    buf->gapIdx += len;
    buf->len += len;
}

static void ctxBufRemove_(CtxBuf *buf, size_t len) {
    if (len > buf->gapIdx) {
        len = buf->gapIdx;
    }

    buf->gapIdx -= len;
    buf->len -= len;

    assert(
        buf->gapIdx == buf->len
        || ucdCh8CPLen(*ctxBufGet_(buf, buf->gapIdx))
    );

    ctxBufShrink_(buf);
}

static void ctxBufSetGapIdx_(CtxBuf *buf, size_t gapIdx) {
    assert(gapIdx <= buf->len);
    assert(
        gapIdx == buf->len
        || ucdCh8CPLen(*ctxBufGet_(buf, gapIdx))
    );
    if (buf->gapIdx == gapIdx) {
        return;
    }

    size_t gapSize = buf->cap - buf->len;

    if (gapIdx > buf->gapIdx) {
        size_t moveSize = gapIdx - buf->gapIdx;
        memmove(
            buf->bytes + buf->gapIdx,
            buf->bytes + buf->gapIdx + gapSize,
            moveSize * sizeof(*buf->bytes)
        );
    } else {
        size_t moveSize = buf->gapIdx - gapIdx;
        memmove(
            buf->bytes + gapIdx + gapSize,
            buf->bytes + gapIdx,
            moveSize * sizeof(*buf->bytes)
        );
    }
    buf->gapIdx = gapIdx;
}

ptrdiff_t ctxNext(const Ctx *ctx, ptrdiff_t idx, UcdCP *outCP) {
    if (idx < 0) {
        idx = 0;
    } else if ((size_t)idx >= ctx->m_buf.len) {
        goto endReached;
    } else {
        idx += ucdCh8RunLen(*ctxBufGet_(&ctx->m_buf, idx));
    }

    if ((size_t)idx >= ctx->m_buf.len) {
        goto endReached;
    }

    if (outCP != NULL) {
        *outCP = ucdCh8ToCP(ctxBufGet_(&ctx->m_buf, idx));
    }
    return idx;

endReached:
    if (outCP != NULL) {
        *outCP = -1;
    }
    return -1;
}

ptrdiff_t ctxPrev(const Ctx *ctx, ptrdiff_t idx, UcdCP *outCP) {
    if (idx == 0 || ctx->m_buf.len == 0) {
        goto endReached;
    } else if (idx < 0) {
        idx = ctx->m_buf.len;
    }
    idx--;
    UcdCh8 *chPtr = ctxBufGet_(&ctx->m_buf, idx);
    while (idx >= 0 && ucdCh8RunLen(*chPtr) == 0) {
        idx--;
        chPtr--;
    }
    if (idx < 0) {
        goto endReached;
    }

    if (outCP != NULL) {
        *outCP = ucdCh8ToCP(chPtr);
    }
    return idx;

endReached:
    if (outCP != NULL) {
        *outCP = -1;
    }
    return -1;
}

ptrdiff_t ctxLineNextStart(const Ctx *ctx, size_t lineIdx, UcdCP *outCP) {
    ptrdiff_t i = ctxLineToIdx_(ctx, lineIdx);
    if (i == ctx->m_buf.len || i < 0) {
        return -1;
    }
    if (outCP) {
        *outCP = ucdCh8ToCP(ctxBufGet_(&ctx->m_buf, i));
    }
    return i;
}

ptrdiff_t ctxLinePrevStart(const Ctx *ctx, size_t lineIdx, UcdCP *outCP) {
    size_t i = ctxLineToIdx_(ctx, lineIdx + 1);
    if (i <= 0) {
        return -1;
    }

    i--;
    while (!ucdCh8RunLen(*ctxBufGet_(&ctx->m_buf, i))) {
        i--;
    }

    if (outCP) {
        *outCP = ucdCh8ToCP(ctxBufGet_(&ctx->m_buf, i));
    }
    return i;
}

ptrdiff_t ctxLineNext(const Ctx *ctx, ptrdiff_t idx, UcdCP *outCP) {
    if (idx < 0) {
        idx = 0;
    } else if ((size_t)idx >= ctx->m_buf.len) {
        goto endReached;
    } else {
        idx += ucdCh8RunLen(*ctxBufGet_(&ctx->m_buf, idx));
    }
    UcdCh8 *chPtr = ctxBufGet_(&ctx->m_buf, idx);

    if ((size_t)idx >= ctx->m_buf.len || *chPtr == '\n') {
        goto endReached;
    }

    if (outCP != NULL) {
        *outCP = ucdCh8ToCP(chPtr);
    }
    return idx;

endReached:
    if (outCP != NULL) {
        *outCP = -1;
    }
    return -1;
}

ptrdiff_t ctxLinePrev(const Ctx *ctx, ptrdiff_t idx, UcdCP *outCP) {
    if (idx == 0) {
        goto endReached;
    } else if (idx < 0) {
        idx = ctx->m_buf.len;
    }
    idx--;
    UcdCh8 *chPtr = ctxBufGet_(&ctx->m_buf, idx);
    while (idx >= 0 && ucdCh8RunLen(*chPtr) == 0) {
        idx--;
        chPtr--;
    }
    if (idx < 0 || *chPtr == '\n') {
        goto endReached;
    }

    if (outCP != NULL) {
        *outCP = ucdCh8ToCP(chPtr);
    }
    return idx;

endReached:
    if (outCP != NULL) {
        *outCP = -1;
    }
    return -1;
}

static void ctxLineAt_(
    const Ctx *ctx,
    size_t idx,
    size_t *outLineNo,
    size_t *outStartIdx
) {
    assert(idx <= ctx->m_buf.len);

    size_t refsLen = ctx->m_lineRefs.len;
    CtxLineRef *refs = ctx->m_lineRefs.items;

    size_t lineCount = 0;
    size_t i = 0;
    size_t lineIdx = 0;

    if (refsLen == 0 || idx < refs[0].idx) {
        goto preciseLine;
    }

    size_t lo = 0;
    size_t hi = refsLen;

    while (lo < hi) {
        size_t mid = (hi + lo) / 2;

        if (refs[mid].idx == idx) {
            i = idx;
            lineCount = refs[mid].lineCount;
            goto preciseLine;
        } else if (refs[mid].idx >= idx) {
            hi = mid;
        } else {
            lo = mid + 1;
        }
    }

    i = refs[lo - 1].idx + 1;
    lineCount = refs[lo - 1].lineCount;

preciseLine:
    for (; i < idx; i++) {
        if (*ctxBufGet_(&ctx->m_buf, i) == '\n') {
            lineCount++;
            lineIdx = i + 1;
        }
    }

    if (outLineNo) {
        *outLineNo = lineCount;
    }
    if (outStartIdx == NULL) {
        return;
    }

    if (lineCount == 0 || lineIdx != 0) {
        *outStartIdx = lineIdx;
        return;
    }
    // The requested line does not begin in the lineRef block, figue out where
    // it starts
    do {
        i--;
    } while (*ctxBufGet_(&ctx->m_buf, i) != '\n');
    *outStartIdx = i + 1;
}

static ptrdiff_t ctxLineToIdx_(const Ctx *ctx, size_t lineNo) {
    if (lineNo == 0) {
        return 0;
    }

    size_t refsLen = ctx->m_lineRefs.len;
    CtxLineRef *refs = ctx->m_lineRefs.items;
    size_t i = 0;
    size_t lineCount = 0;
    size_t endIdx = refsLen == 0 ? ctx->m_buf.len : refs[0].idx;

    if (refsLen == 0 || refs[0].lineCount >= lineNo) {
        goto preciseIdx;
    }

    size_t lo = 0;
    size_t hi = refsLen;

    while (lo < hi) {
        size_t mid = (hi + lo) / 2;

        if (refs[mid].lineCount >= lineNo) {
            hi = mid;
        } else {
            lo = mid + 1;
        }
    }

    i = refs[lo - 1].idx;
    lineCount = refs[lo - 1].lineCount;
    if (lo == refsLen) {
        endIdx = ctx->m_buf.len;
    } else {
        endIdx = refs[lo].idx;
    }

preciseIdx:
    for (; i < endIdx; i++) {
        if (*ctxBufGet_(&ctx->m_buf, i) != '\n') {
            continue;
        }
        lineCount++;
        if (lineCount == lineNo) {
            return i + 1;
        }
    }

    // The line does not exist
    return -1;
}

void ctxAppend(Ctx *ctx, const UcdCh8 *data, size_t len) {
    if (len == 0) {
        return;
    }

    ctx->edited = true;

    size_t lineStart = 0;
    bool ignoreNL = !ctx->multiline;

    CtxBuf *buf = &ctx->m_buf;
    size_t initialLen = buf->len;
    uint16_t lastBlockSize = ctx->m_lineRefs.len == 0
        ? initialLen
        : initialLen - ctx->m_lineRefs.items[ctx->m_lineRefs.len - 1].idx - 1;
    size_t lineCount;
    ctxLineAt_(ctx, initialLen, &lineCount, NULL);

    ctxBufSetGapIdx_(buf, initialLen);

    for (size_t i = 0; i < len; i++) {
        if (data[i] == '\n' && !ignoreNL) {
            ctxBufInsert_(buf, &data[lineStart], i - lineStart);
            ctxBufInsert_(buf, (const UcdCh8 *)"\n", 1);
            lineStart = i + 1;
            lineCount++;
        } else if (data[i] == '\r' || data[i] == '\n') {
            ctxBufInsert_(buf, &data[lineStart], i - lineStart);
            lineStart = i + 1;
            continue;
        }
        lastBlockSize++;
        if (lastBlockSize == lineRefMaxGap_) {
            arrAppend(
                &ctx->m_lineRefs,
                (CtxLineRef){ .idx = i + initialLen, .lineCount = lineCount }
            );
            lastBlockSize = 0;
        }
    }

    if (lineStart < len) {
        ctxBufInsert_(buf, &data[lineStart], len - lineStart);
    }
}

static size_t ctxCursorAt_(Ctx *ctx, size_t idx) {
    size_t hi = ctx->m_cursors.len;
    size_t lo = 0;
    size_t *cursors = ctx->m_cursors.items;

    while (lo < hi) {
        size_t mid = (lo + hi) / 2;
        if (cursors[mid] == idx) {
            return mid;
        } else if (cursors[mid] > idx) {
            hi = mid;
        } else {
            lo = mid + 1;
        }

    }

    return lo;
}

static void ctxCursorAdd_(Ctx *ctx, size_t idx) {
    size_t curIdx = ctxCursorAt_(ctx, idx);
    if (curIdx >= ctx->m_cursors.len) {
        arrAppend(&ctx->m_cursors, idx);
        return;
    // Do not duplicate cursors
    } else if (ctx->m_cursors.items[curIdx] != idx) {
        arrInsert(&ctx->m_cursors, curIdx, idx);
    }
}

static void ctxCursorRemove_(Ctx *ctx, size_t idx) {
    size_t curIdx = ctxCursorAt_(ctx, idx);
    if (curIdx < ctx->m_cursors.len && ctx->m_cursors.items[curIdx] == idx) {
        arrRemove(&ctx->m_cursors, curIdx);
    }
}

static void ctxCursorReplace_(Ctx *ctx, size_t old, size_t new) {
    size_t oldIdx = ctxCursorAt_(ctx, old);
    size_t newIdx = ctxCursorAt_(ctx, new);
    size_t *cursors = ctx->m_cursors.items;

    // If `new` is already a cursor
    if (newIdx < ctx->m_cursors.len && cursors[newIdx] == new) {
        ctxCursorRemove_(ctx, old);
        return;
    }

    // If `old` does not exist
    if (oldIdx >= ctx->m_cursors.len || cursors[oldIdx] != old) {
        ctxCursorAdd_(ctx, new);
        return;
    }

    if (oldIdx == newIdx) {
        cursors[oldIdx] = new;
    } else if (oldIdx < newIdx) {
        memmove(
            &cursors[oldIdx],
            &cursors[oldIdx + 1],
            sizeof(*cursors) * (newIdx - oldIdx - 1)
        );
        cursors[newIdx - 1] = new;
    } else {
        memmove(
            &cursors[newIdx + 1],
            &cursors[newIdx],
            sizeof(*cursors) * (oldIdx - newIdx - 1)
        );
        cursors[newIdx] = new;
    }
}

#if 0

#include <assert.h>
#include <errno.h>
#include <string.h>
#include "nv_array.h"
#include "nv_context.h"
#include "nv_udb.h"

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

# endif
