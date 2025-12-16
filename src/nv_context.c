#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "nv_context.h"
#include "nv_array.h"
#include "nv_mem.h"
#include "nv_unicode.h"
#include "nv_udb.h"

#ifndef _lineRefMaxGap
#define _lineRefMaxGap 4096
#endif // !_lineRefMaxGap

// Get from the gap buffer
static inline UcdCh8 *_ctxBufGet(const CtxBuf *buf, size_t idx);
// Reserve space in the gap buffer
static void _ctxBufReserve(CtxBuf *buf, size_t amount);
// Shrkin if too much space is empty
static void _ctxBufShrink(CtxBuf *buf);
// Insert at the gapIdx and move it
static void _ctxBufInsert(CtxBuf *buf, const UcdCh8 *text, size_t len);
// Remove before the gap index and move it
static void _ctxBufRemove(CtxBuf *buf, size_t len);
// Change the gap index
static void _ctxBufSetGapIdx(CtxBuf *buf, size_t gapIdx);

// Get the index of the first character of a line, it may be equal to the length
// of the buffer
static ptrdiff_t _ctxLineStart(const Ctx *ctx, size_t lineNo);
// Get the index of the last characer of a line, it may be equal to the length
// of the buffer
static ptrdiff_t _ctxLineEnd(const Ctx *ctx, size_t lineNo);

// Get the unicode character after `idx`
static UcdCP _ctxGetChAfter(const Ctx *ctx, size_t idx);
// Get the unicode character before `idx`
static UcdCP _ctxGetChBefore(const Ctx *ctx, size_t idx);

static size_t _ctxFindNextWordStart(const Ctx *ctx, size_t idx);
static size_t _ctxFindNextWordEnd(const Ctx *ctx, size_t idx);
static size_t _ctxFindPrevWordStart(const Ctx *ctx, size_t idx);
static size_t _ctxFindPrevWordEnd(const Ctx *ctx, size_t idx);

// Replace the text in [start, end) with the contents of buf
static void _ctxReplace(
    Ctx *ctx,
    size_t start,
    size_t end,
    const UcdCh8 *buf,
    size_t len
);

// Get the index of the cursor at idx or of the cursor on where it should be
// inserted
static size_t _ctxCurAt(const Ctx *ctx, size_t idx);
static void _ctxCurAddEx(Ctx *ctx, size_t idx, size_t col);
// Return `true` if the new cursor already exists
static bool _ctxCurMove(Ctx *ctx, size_t old, size_t new);
static bool _ctxCurMoveEx(Ctx *ctx, size_t old, size_t new, size_t newCol);

// Find the index at position `line`, `col`.
// The line must match exactly, the column is the closest to `col`.

// Join cursor selections in the _sels array
static void _ctxSelJoin(Ctx *ctx);

void ctxInit(Ctx *ctx, bool multiline) {
    ctx->_refs = (CtxRefs){ 0 };
    ctx->_sels = (CtxSelections){ 0 };

    ctx->_buf = (CtxBuf) {
        .bytes = NULL,
        .len = 0,
        .cap = 0,
        .gapIdx = 0
    };

    ctx->cursors = (CtxCursors){ 0 };
    ctx->_selecting = false;
    ctx->edited = false;
    ctx->multiline = multiline;
    ctx->tabStop = 8;
}

void ctxDestroy(Ctx *ctx) {
    arrClear(&ctx->_refs);
    arrClear(&ctx->_sels);
    arrClear(&ctx->cursors);

    memFree(ctx->_buf.bytes);

    // Keep the context in a valid state after deletion
    ctxInit(ctx, ctx->multiline);
}

static inline UcdCh8 *_ctxBufGet(const CtxBuf *buf, size_t idx) {
    assert(idx < buf->len);
    if (idx >= buf->gapIdx) {
        size_t gapSize = buf->cap - buf->len;
        idx += gapSize;
    }
    return &buf->bytes[idx];
}

static void _ctxBufReserve(CtxBuf *buf, size_t amount) {
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

static void _ctxBufShrink(CtxBuf *buf) {
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

static void _ctxBufInsert(CtxBuf *buf, const UcdCh8 *text, size_t len) {
    _ctxBufReserve(buf, len);
    memcpy(buf->bytes + buf->gapIdx, text, len * sizeof(*text));
    buf->gapIdx += len;
    buf->len += len;
}

static void _ctxBufRemove(CtxBuf *buf, size_t len) {
    if (len > buf->gapIdx) {
        len = buf->gapIdx;
    }

    buf->gapIdx -= len;
    buf->len -= len;

    assert(
        buf->gapIdx == buf->len
        || ucdCh8IsStart(*_ctxBufGet(buf, buf->gapIdx))
    );

    _ctxBufShrink(buf);
}

static void _ctxBufSetGapIdx(CtxBuf *buf, size_t gapIdx) {
    assert(gapIdx <= buf->len);
    assert(
        gapIdx == buf->len
        || ucdCh8IsStart(*_ctxBufGet(buf, gapIdx))
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

// Get the block where line lineNo starts, -1 means the beginning of the file
static ptrdiff_t _ctxGetLineRefBlock(const Ctx *ctx, size_t lineNo) {
    size_t refsLen = ctx->_refs.len;
    CtxRef *refs = ctx->_refs.items;

    if (lineNo == 0 || refsLen == 0 || refs[0].line >= lineNo) {
        return -1;
    }

    size_t lo = 0;
    size_t hi = refsLen;

    while (lo < hi) {
        size_t mid = (hi + lo) / 2;

        if (refs[mid].line >= lineNo) {
            hi = mid;
        } else {
            lo = mid + 1;
        }
    }

    return lo - 1;
}

// Get the block that contains idx
static ptrdiff_t _ctxGetIdxRefBlock(const Ctx *ctx, size_t idx) {
    size_t refsLen = ctx->_refs.len;
    CtxRef *refs = ctx->_refs.items;

    if (idx == 0 || refsLen == 0 || refs[0].idx > idx) {
        return -1;
    }

    size_t lo = 0;
    size_t hi = refsLen;

    while (lo < hi) {
        size_t mid = (hi + lo) / 2;

        if (refs[mid].idx == idx) {
            return mid;
        } else if (refs[mid].idx > idx) {
            hi = mid;
        } else {
            lo = mid + 1;
        }
    }

    return lo - 1;
}

static ptrdiff_t _ctxLineStart(const Ctx *ctx, size_t lineNo) {
    if (lineNo == 0) {
        return 0;
    }

    ptrdiff_t refsIdx = _ctxGetLineRefBlock(ctx, lineNo);
    size_t i;
    size_t lineCount;

    if (refsIdx == -1) {
        i = 0;
        lineCount = 0;
    } else {
        i = ctx->_refs.items[refsIdx].idx;
        lineCount = ctx->_refs.items[refsIdx].line;
    }

    size_t endIdx = (size_t)(refsIdx + 1) < ctx->_refs.len
        ? ctx->_refs.items[refsIdx + 1].idx
        : ctx->_buf.len;

    // Get the precise index
    for (; i < endIdx; i++) {
        if (*_ctxBufGet(&ctx->_buf, i) != '\n') {
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

static ptrdiff_t _ctxLineEnd(const Ctx *ctx, size_t lineNo) {
    ptrdiff_t refsIdx = _ctxGetLineRefBlock(ctx, lineNo + 1);
    size_t i;
    size_t lineCount;

    if (refsIdx == -1) {
        i = 0;
        lineCount = 0;
    } else {
        i = ctx->_refs.items[refsIdx].idx;
        lineCount = ctx->_refs.items[refsIdx].line;
    }

    size_t endIdx = (size_t)(refsIdx + 1) < ctx->_refs.len
        ? ctx->_refs.items[refsIdx + 1].idx
        : ctx->_buf.len;

    for (; i < endIdx; i++) {
        if (*_ctxBufGet(&ctx->_buf, i) != '\n') {
            continue;
        }
        lineCount++;
        if (lineCount > lineNo) {
            break;
        }
    }

    if (lineCount < lineNo) {
        return  -1;
    } else {
        return i;
    }
}

void ctxPosAt(const Ctx *ctx, size_t idx, size_t *outLine, size_t *outCol) {
    assert(idx <= ctx->_buf.len);

    size_t refsLen = ctx->_refs.len;
    CtxRef *refs = ctx->_refs.items;

    size_t line = 0;
    size_t startIdx = 0;
    size_t col = 0;

    ptrdiff_t refIdx = _ctxGetIdxRefBlock(ctx, idx);
    if (refIdx >= 0) {
        startIdx = refs[refIdx].idx;
        line = refs[refIdx].line;
        col = refs[refIdx].col;
    }

    for (size_t i = startIdx; i < idx; i++) {
        if (*_ctxBufGet(&ctx->_buf, i) == '\n') {
            line++;
            startIdx = i + 1;
            col = 0;
        }
    }

    if (outLine) {
        *outLine = line;
    }

    if (outCol == NULL) {
        return;
    }

    UcdCP cp = 0;
    for (
        ptrdiff_t j = ctxNext(ctx, (ptrdiff_t)startIdx - 1, &cp);
        j >= 0;
        j = ctxNext(ctx, j, &cp)
    ) {
        if (j >= idx) {
            break;
        }
        col += ucdCPWidth(cp, ctx->tabStop, col);
    }
    *outCol = col;
}

ptrdiff_t ctxIdxAt(
    const Ctx *ctx,
    size_t line,
    size_t col,
    size_t *outTrueCol
) {
    ptrdiff_t refsIdx = _ctxGetLineRefBlock(ctx, line);
    size_t i;
    size_t refLine;
    size_t refCol;
    UcdCP cp;

    if (refsIdx == -1) {
        i = 0;
        refLine = 0;
        refCol = 0;
    } else {
        refsIdx++;
        while (
            ctx->_refs.items[refsIdx].line == line
            && ctx->_refs.items[refsIdx].col <= col
        ) {
            refsIdx++;
        }
        refsIdx--;

        i = ctx->_refs.items[refsIdx].idx;
        refLine = ctx->_refs.items[refsIdx].line;
        refCol = ctx->_refs.items[refsIdx].col;
    }

    size_t endIdx = (size_t)(refsIdx + 1) < ctx->_refs.len
        ? ctx->_refs.items[refsIdx + 1].idx
        : ctx->_buf.len;

    // Get the precise index
    for (; refLine < line && i < endIdx; i++) {
        if (*_ctxBufGet(&ctx->_buf, i) != '\n') {
            continue;
        }
        refLine++;
        refCol = 0;
        if (refLine == line) {
            i++;
            break;
        }
    }

    if (refLine != line) {
        // The line does not exist
        return -1;
    }

    for (i = ctxNext(ctx, i - 1, &cp); i != -1; i = ctxNext(ctx, i, &cp)) {
        if (cp == '\n') {
            break;
        }
        uint8_t chWidth = ucdCPWidth(cp, ctx->tabStop, refCol);
        if (refCol + chWidth < col) {
            refCol += chWidth;
            continue;
        }
        // If it is closer to the end of the character do another iteration
        if ((ptrdiff_t)(refCol + chWidth - col) < (ptrdiff_t)(col - refCol)) {
            refCol += chWidth;
            continue;
        }
        break;
    }
    if (i == -1) {
        i = ctx->_buf.len;
    }
    if (outTrueCol != NULL) {
        *outTrueCol = refCol;
    }
    return i;
}

ptrdiff_t ctxNext(const Ctx *ctx, ptrdiff_t idx, UcdCP *outCP) {
    if (idx < 0) {
        idx = 0;
    } else if ((size_t)idx < ctx->_buf.len) {
        uint8_t offset = ucdCh8RunLen(*_ctxBufGet(&ctx->_buf, idx));
        // If idx is not on a character boundary find the next one
        if (offset == 0) {
            idx++;
            while (
                idx < ctx->_buf.len
                && !ucdCh8IsStart(*_ctxBufGet(&ctx->_buf, idx))
            ) {
                idx++;
            }
        } else {
            idx += offset;
        }
    }

    if ((size_t)idx >= ctx->_buf.len) {
        if (outCP != NULL) {
            *outCP = -1;
        }
        return -1;
    }

    if (outCP != NULL) {
        *outCP = ucdCh8ToCP(_ctxBufGet(&ctx->_buf, idx));
    }
    return idx;
}

ptrdiff_t ctxPrev(const Ctx *ctx, ptrdiff_t idx, UcdCP *outCP) {
    if (idx == 0 || ctx->_buf.len == 0) {
        goto endReached;
    } else if (idx < 0) {
        idx = ctx->_buf.len;
    }
    idx--;
    UcdCh8 *chPtr = _ctxBufGet(&ctx->_buf, idx);
    while (idx >= 0 && !ucdCh8IsStart(*chPtr)) {
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
    ptrdiff_t i = _ctxLineStart(ctx, lineIdx);
    if (i == ctx->_buf.len || i < 0) {
        goto noLine;
    }
    if (*_ctxBufGet(&ctx->_buf, i) == '\n') {
        goto noLine;
    }
    if (outCP) {
        *outCP = ucdCh8ToCP(_ctxBufGet(&ctx->_buf, i));
    }
    return i;

noLine:
    if (outCP) {
        *outCP = -1;
    }
    return -1;
}

ptrdiff_t ctxLinePrevStart(const Ctx *ctx, size_t lineIdx, UcdCP *outCP) {
    ptrdiff_t i = _ctxLineEnd(ctx, lineIdx);
    if (i <= 0) {
        goto noLine;
    }
    i--;
    UcdCh8 *chPtr = _ctxBufGet(&ctx->_buf, i);
    if (*chPtr == '\n') {
        goto noLine;
    }
    while (i >= 0 && !ucdCh8IsStart(*chPtr)) {
        i--;
        chPtr--;
    }
    if (outCP) {
        *outCP = ucdCh8ToCP(chPtr);
    }
    return i;

noLine:
    if (outCP) {
        *outCP = -1;
    }
    return -1;
}

ptrdiff_t ctxLineNext(const Ctx *ctx, ptrdiff_t idx, UcdCP *outCP) {
    if (idx < 0) {
        idx = 0;
    }

    if ((size_t)idx < ctx->_buf.len) {
        idx += ucdCh8RunLen(*_ctxBufGet(&ctx->_buf, idx));
    }

    if ((size_t)idx >= ctx->_buf.len) {
        goto endReached;
    }

    UcdCh8 *chPtr = _ctxBufGet(&ctx->_buf, idx);

    if (*chPtr == '\n') {
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
    if (idx < 0) {
        idx = ctx->_buf.len;
    }
    if (idx == 0) {
        goto endReached;
    }
    idx--;
    UcdCh8 *chPtr = _ctxBufGet(&ctx->_buf, idx);
    while (idx >= 0 && !ucdCh8IsStart(*chPtr)) {
        idx--;
        chPtr = _ctxBufGet(&ctx->_buf, idx);
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

static UcdCP _ctxGetChAfter(const Ctx *ctx, size_t idx) {
    if (idx >= ctx->_buf.len) {
        return -1;
    }
    return ucdCh8ToCP(_ctxBufGet(&ctx->_buf, idx));
}

static UcdCP _ctxGetChBefore(const Ctx *ctx, size_t idx) {
    UcdCP cp;
    (void)ctxPrev(ctx, idx, &cp);
    return cp;
}

StrView ctxGetContent(Ctx *ctx) {
    _ctxBufSetGapIdx(&ctx->_buf, ctx->_buf.len);
    return (StrView) { .buf = ctx->_buf.bytes, .len = ctx->_buf.len };
}

static size_t _ctxFindNextWordStart(const Ctx *ctx, size_t idx) {
    if (idx >= ctx->_buf.len) {
        return ctx->_buf.len;
    }
    ptrdiff_t i = idx;
    UcdCP cp = _ctxGetChAfter(ctx, idx);

    if (ucdIsCPAlphanumeric(cp)) {
        for (
            i = ctxNext(ctx, i, &cp);
            i != -1 && ucdIsCPAlphanumeric(cp);
            i = ctxNext(ctx, i, &cp)
        ) { }
    } else {
        for (
            i = ctxNext(ctx, i, &cp);
            i != -1 && !ucdIsCPWhiteSpace(cp) && !ucdIsCPAlphanumeric(cp);
            i = ctxNext(ctx, i, &cp)
        ) { }
    }

    // Skip white space
    for (; i != -1 && ucdIsCPWhiteSpace(cp); i = ctxNext(ctx, i, &cp)) { }

    if (i == -1) {
        i = ctx->_buf.len;
    }
    return (size_t)i;
}

static size_t _ctxFindNextWordEnd(const Ctx *ctx, size_t idx) {
    if (idx >= ctx->_buf.len) {
        return ctx->_buf.len;
    }
    ptrdiff_t i = idx;
    UcdCP cp = _ctxGetChAfter(ctx, idx);

    // Skip white space
    for (; i != -1 && ucdIsCPWhiteSpace(cp); i = ctxNext(ctx, i, &cp)) { }

    if (ucdIsCPAlphanumeric(cp)) {
        for (
            i = ctxNext(ctx, i, &cp);
            i != -1 && ucdIsCPAlphanumeric(cp);
            i = ctxNext(ctx, i, &cp)
        ) { }
    } else {
        for (
            i = ctxNext(ctx, i, &cp);
            i != -1 && !ucdIsCPWhiteSpace(cp) && !ucdIsCPAlphanumeric(cp);
            i = ctxNext(ctx, i, &cp)
        ) { }
    }

    if (i == -1) {
        i = ctx->_buf.len;
    }
    return (size_t)i;
}

static size_t _ctxFindPrevWordStart(const Ctx *ctx, size_t idx) {
    if (idx == 0) {
        return 0;
    }
    ptrdiff_t i = idx;
    UcdCP cp = _ctxGetChBefore(ctx, idx);

    // Skip white space
    for (; i != -1 && ucdIsCPWhiteSpace(cp); i = ctxPrev(ctx, i, &cp)) { }

    if (ucdIsCPAlphanumeric(cp) && i != -1) {
        for (
            i = ctxPrev(ctx, i, &cp);
            i != -1 && ucdIsCPAlphanumeric(cp);
            i = ctxPrev(ctx, i, &cp)
        ) { }
    } else if (i != -1) {
        for (
            i = ctxPrev(ctx, i, &cp);
            i != -1 && !ucdIsCPWhiteSpace(cp) && !ucdIsCPAlphanumeric(cp);
            i = ctxPrev(ctx, i, &cp)
        ) { }
    }
    if (i == -1) {
        i = 0;
    } else {
        i = ctxNext(ctx, i, NULL);
    }
    return i;
}

static size_t _ctxFindPrevWordEnd(const Ctx *ctx, size_t idx) {
    if (idx == 0) {
        return 0;
    }
    ptrdiff_t i = idx;
    UcdCP cp = _ctxGetChBefore(ctx, idx);

    if (ucdIsCPAlphanumeric(cp) && i != -1) {
        for (
            i = ctxPrev(ctx, i, &cp);
            i != -1 && ucdIsCPAlphanumeric(cp);
            i = ctxPrev(ctx, i, &cp)
        ) { }
    } else if (i != -1) {
        for (
            i = ctxPrev(ctx, i, &cp);
            i != -1 && !ucdIsCPWhiteSpace(cp) && !ucdIsCPAlphanumeric(cp);
            i = ctxPrev(ctx, i, &cp)
        ) { }
    }

    // Skip white space
    for (; i != -1 && ucdIsCPWhiteSpace(cp); i = ctxPrev(ctx, i, &cp)) { }

    if (i == -1) {
        i = 0;
    } else {
        i = ctxNext(ctx, i, NULL);
    }
    return i;
}

static void _ctxReplaceUpdateCursors(
    Ctx *ctx,
    size_t start,
    size_t end,
    ptrdiff_t lenDiff
) {
    bool selecting = ctx->_selecting;
    CtxCursor *cursors = ctx->cursors.items;
    size_t changedLine;
    ctxPosAt(ctx, end + lenDiff, &changedLine, NULL);
    size_t lastBaseIdxCalc = _ctxLineEnd(ctx, changedLine);

    // Move the active selections, if selecting
    // When not selecting the value of _selStart is assumed to be invalid
    for (size_t i = 0; i < ctx->cursors.len; i++) {
        CtxCursor *cur = &cursors[i];
        if (cur->idx == end && i > 0 && cursors[i - 1].idx == end + lenDiff) {
            arrRemove(&ctx->cursors, i);
            i--;
            continue;
        }

        if (cur->idx >= end) {
            cur->idx += lenDiff;
            if (cur->idx <= lastBaseIdxCalc) {
                ctxPosAt(ctx, cur->idx, NULL, &cur->baseCol);
            }
        }
        if (!selecting || cur->_selStart <= start) {
            continue;
        } else if (cur->_selStart >= end) {
            cur->_selStart += lenDiff;
        // From here we know that _selStart was inside the span
        } else if (cur->idx <= start) {
            cur->_selStart = start;
        } else {
            cur->_selStart = end + lenDiff;
        }
    }
}

static void _ctxReplaceUpdateSelections(
    Ctx *ctx,
    size_t start,
    size_t end,
    ptrdiff_t lenDiff
) {
    bool mayJoin = -lenDiff == end - start;
    for (size_t i = 0; i < ctx->_sels.len; i++) {
        CtxSelection *sel = &ctx->_sels.items[i];
        if (sel->endIdx <= start) {
            continue;
        } else if (sel->startIdx >= end) {
            sel->startIdx += lenDiff;
            sel->endIdx += lenDiff;
        } else if (sel->startIdx >= start && sel->endIdx <= end) {
            arrRemove(&ctx->_sels, i);
            i--;
            continue;
        } else if (sel->startIdx < start && sel->endIdx > end) {
            sel->endIdx += lenDiff;
        } else if (sel->startIdx < start) {
            sel->endIdx = start;
        } else {
            sel->startIdx = end + lenDiff;
            sel->endIdx += lenDiff;
        }
        if (!mayJoin || sel->startIdx < start) {
            continue;
        }
        if (i > 0 && sel->startIdx == ctx->_sels.items[i - 1].endIdx) {
            ctx->_sels.items[i - 1].endIdx = sel->endIdx;
            arrRemove(&ctx->_sels, i);
            i--;
        }
        mayJoin = false;
    }
}

// Make the span of the block from [refBlock - 1, refBlock] reasonable
void _ctxReplaceBalanceRefBlocks(Ctx *ctx, size_t refBlock) {
    const size_t minWidth = _lineRefMaxGap / 2;
    const size_t maxWidth = _lineRefMaxGap + minWidth;

    size_t blockStart = refBlock > 0 ? ctx->_refs.items[refBlock - 1].idx : 0;
    size_t blockEnd = refBlock < ctx->_refs.len
        ? ctx->_refs.items[refBlock].idx
        : ctx->_buf.len;
    size_t nextBlockEnd = refBlock + 1 < ctx->_refs.len
        ? ctx->_refs.items[refBlock + 1].idx
        : ctx->_buf.len;
    bool isLastBlock = refBlock >= ctx->_refs.len;

    size_t width = blockEnd - blockStart;
    size_t fullWidth = nextBlockEnd - blockStart;

    if (width >= minWidth && width <= maxWidth) {
        return;
    }

    if (
        width > maxWidth
        && (isLastBlock || fullWidth > 2 * _lineRefMaxGap)
    ) {
        // Divide both to avoid overflow
        size_t newIdx = blockStart / 2 + blockEnd / 2;
        size_t line, col;
        ctxPosAt(ctx, newIdx, &line, &col);
        arrInsert(
            &ctx->_refs,
            refBlock,
            (CtxRef){ .idx = newIdx, .line = line, .col = col }
        );
    } else if (!isLastBlock && fullWidth <= maxWidth) {
        arrRemove(&ctx->_refs, refBlock);
    } else if (!isLastBlock) {
        size_t newIdx = blockStart / 2 + nextBlockEnd / 2;
        size_t line, col;
        ctxPosAt(ctx, newIdx, &line, &col);
        ctx->_refs.items[refBlock].idx = newIdx;
        ctx->_refs.items[refBlock].line = line;
        ctx->_refs.items[refBlock].col = col;
    }
}

static void _ctxReplace(
    Ctx *ctx,
    size_t start,
    size_t end,
    const UcdCh8 *data,
    size_t len
) {
    assert(start <= end);
    assert(end <= ctx->_buf.len);

    ctx->edited = true;

    // Move all cursors inside the span to the end
    size_t curIdx = _ctxCurAt(ctx, start) + 1;
    if (curIdx < ctx->cursors.len) {
        while (ctx->cursors.items[curIdx].idx < end) {
            ctxCurMove(ctx, ctx->cursors.items[curIdx].idx, end);
        }
    }

    // Ref blocks are inserted while copying the text, any gaps too small or
    // too large at the end are fixed later

    size_t spanStart = 0;
    uint8_t tabStop = ctx->tabStop;
    bool ignoreNL = !ctx->multiline;
    CtxBuf *buf = &ctx->_buf;
    CtxRefs *refs = &ctx->_refs;
    ptrdiff_t lenDiff = (ptrdiff_t)len - (ptrdiff_t)(end - start);
    ptrdiff_t refBlock = _ctxGetIdxRefBlock(ctx, start);
    size_t lastBlockIdx = refBlock < 0 ? 0 : refs->items[refBlock].idx;
    refBlock++; // Ensures that refBlock is positive

    // Keep track of line and column while iterating because the ref cache is
    // not valid after _ctxBufRemove
    size_t line, col;
    ctxPosAt(ctx, start, &line, &col);
    size_t prevLine, prevCol;
    ctxPosAt(ctx, end, &prevLine, &prevCol);

    _ctxBufSetGapIdx(buf, end);
    _ctxBufRemove(buf, end - start);
    _ctxBufReserve(buf, len);

    StrView sv = {
        .buf = data,
        .len = len
    };

    UcdCP cp;
    ptrdiff_t idx;
    for (
        idx = strViewNext(&sv, -1, &cp);
        idx >= 0;
        idx = strViewNext(&sv, idx, &cp)
    ) {
        if (cp == '\r' || (cp == '\n' && ignoreNL)) {
            _ctxBufInsert(buf, &data[spanStart], idx - spanStart);
            spanStart = idx + 1;
            lenDiff--;
            continue;
        } else if (cp == '\n') {
            line++;
            col = 0;
        } else {
            col += ucdCPWidth(cp, tabStop, col);
        }

        if (idx - lastBlockIdx < _lineRefMaxGap) {
            continue;
        }

        _ctxBufInsert(buf, &data[spanStart], idx - spanStart + 1);
        spanStart = idx + 1;
        lastBlockIdx = idx;
        arrInsert(
            refs,
            refBlock,
            (CtxRef){ .idx = idx, .line = line, .col = col }
        );
        refBlock++;
    }

    if (spanStart < len) {
        _ctxBufInsert(buf, &data[spanStart], len - spanStart);
    }

    // Remove all blocks that were inside the modified span
    while (
        (size_t)refBlock < refs->len
        && refs->items[refBlock].idx < end
    ) {
        arrRemove(refs, refBlock);
    }

    ptrdiff_t lineDiff = line - prevLine;
    for (size_t i = refBlock, n = refs->len; i < n; i++) {
        refs->items[i].idx += lenDiff;
        refs->items[i].line += lineDiff;
    }

    ptrdiff_t colDiff = col - prevCol;
    size_t tabIdx = buf->len;
    ptrdiff_t lineEnd = _ctxLineEnd(ctx, line);
    assert(lineEnd >= 0);
    if (tabStop != 0 && colDiff % tabStop != 0 && end + lenDiff < buf->len) {
        UcdCh8 *s = _ctxBufGet(buf, end + lenDiff);
        UcdCh8 *p = memchr(s, '\t', lineEnd - end - lenDiff);
        if (p != NULL) {
            tabIdx = p - s + end + lenDiff;
        }
    }

    for (size_t i = refBlock, n = refs->len; i < n; i++) {
        CtxRef *ref = &refs->items[i];
        if (colDiff == 0 || ref->idx > lineEnd) {
            break;
        } else if (ref->idx < tabIdx) {
            ref->col += colDiff;
            continue;
        }
        size_t width;
        ctxPosAt(ctx, tabIdx, NULL, &width);
        colDiff += ((width - colDiff) % tabStop) - (width % tabStop);
        ref->col += colDiff;
        tabIdx = buf->len;
    }

    _ctxReplaceBalanceRefBlocks(ctx, refBlock);
    _ctxReplaceUpdateSelections(ctx, start, end, lenDiff);
    _ctxReplaceUpdateCursors(ctx, start, end, lenDiff);
}

void ctxAppend(Ctx *ctx, const UcdCh8 *data, size_t len) {
    if (len == 0) {
        return;
    }

    ctx->edited = true;

    size_t spanStart = 0;
    bool ignoreNL = !ctx->multiline;
    CtxBuf *buf = &ctx->_buf;
    uint16_t lastBlockSize = ctx->_refs.len == 0
        ? buf->len
        : buf->len - ctx->_refs.items[ctx->_refs.len - 1].idx;

    _ctxBufSetGapIdx(buf, buf->len);

    for (size_t i = 0; i < len; i++) {
        if (data[i] == '\r' || (data[i] == '\n' && ignoreNL)) {
            _ctxBufInsert(buf, &data[spanStart], i - spanStart);
            spanStart = i + 1;
            continue;
        }

        // Do not insert blocks in the middle of a UTF8 sequence
        if (
            ++lastBlockSize < _lineRefMaxGap
            || (i + 1 < len && !ucdCh8IsStart(data[i + 1]))
        ) {
            continue;
        }

        _ctxBufInsert(buf, &data[spanStart], i - spanStart + 1);
        spanStart = i + 1;
        size_t line, col;
        ctxPosAt(ctx, ctx->_buf.len, &line, &col);
        arrAppend(
            &ctx->_refs,
            (CtxRef){ .idx = ctx->_buf.len, .line = line, .col = col }
        );
        lastBlockSize = 0;
    }

    if (spanStart < len) {
        _ctxBufInsert(buf, &data[spanStart], len - spanStart);
    }
}

typedef Arr(const UcdCh8 *) InsertLines;

static void _ctxInsertRepSels(
    Ctx *ctx,
    const UcdCh8 *data,
    size_t len,
    InsertLines *lines
) {
    CtxSelections *sels = &ctx->_sels;
    size_t selsLen = sels->len;
    if (selsLen == 0) {
        return;
    }

    bool useLines = lines != NULL && lines->len == selsLen + 1;
    for (size_t i = 0; i < selsLen; i++) {
        size_t idx = selsLen - i - 1;
        CtxSelection sel = sels->items[idx];
        arrRemove(sels, idx);
        if (useLines) {
            _ctxReplace(
                ctx,
                sel.startIdx,
                sel.endIdx,
                lines->items[idx],
                lines->items[idx + 1] - lines->items[idx] - 1
            );
        } else {
            _ctxReplace(ctx, sel.startIdx, sel.endIdx, data, len);
        }
    }
}

static void _ctxInsertCursors(
    Ctx *ctx,
    const UcdCh8 *data,
    size_t len,
    InsertLines *lines
) {
    if (len == 0) {
        return;
    }

    CtxCursors *cursors = &ctx->cursors;
    size_t cursorsLen = cursors->len;
    if (cursorsLen == 0) {
        return;
    }

    bool useLines = lines != NULL && lines->len == cursorsLen + 1;
    for (size_t i = 0; i < cursorsLen; i++) {
        CtxCursor *cur = &cursors->items[i];
        if (useLines) {
            _ctxReplace(
                ctx,
                cur->idx,
                cur->idx,
                lines->items[i],
                lines->items[i + 1] - lines->items[i] - 1
            );
        } else {
            _ctxReplace(ctx, cur->idx, cur->idx, data, len);
        }
    }
}

void ctxInsert(Ctx *ctx, const UcdCh8 *data, size_t len) {
    bool wasSelecting = ctx->_selecting;
    if (ctx->_selecting) {
        ctxSelEnd(ctx);
    }
    if (ctx->_sels.len == 0 && ctx->cursors.len == 0) {
        return;
    }

    if (ctx->_sels.len == 1) {
        _ctxInsertRepSels(ctx, data, len, NULL);
        return;
    } else if (ctx->cursors.len == 1 && !wasSelecting) {
        _ctxInsertCursors(ctx, data, len, NULL);
        return;
    }

    InsertLines lines = { 0 };
    arrAppend(&lines, data);
    const UcdCh8 *p = data;
    const UcdCh8 *end = data + len;
    while (p < end) {
        if (*p++ == '\n') {
            arrAppend(&lines, p);
        }
    }
    if (lines.len == 0 || lines.items[lines.len - 1] != p) {
        arrAppend(&lines, end + 1);
    }
    if (ctx->_sels.len != 0 || wasSelecting) {
        _ctxInsertRepSels(ctx, data, len, &lines);
    } else {
        _ctxInsertCursors(ctx, data, len, &lines);
    }
    arrClear(&lines);
}

static size_t cpToUTF8Filtered_(UcdCP cp, bool allowLF, UcdCh8 *outBuf) {
    if (cp < 0 || cp > ucdCPMax) {
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

static void _ctxRemoveSelections(Ctx *ctx) {
    for (size_t i = ctx->_sels.len; i > 0; i--) {
        CtxSelection sel = ctx->_sels.items[i - 1];
        _ctxReplace(ctx, sel.startIdx, sel.endIdx, NULL, 0);
    }
    arrClear(&ctx->_sels);
}

void ctxRemoveBack(Ctx *ctx) {
    if (ctx->_selecting) {
        ctxSelEnd(ctx);
    }

    if (ctx->_sels.len == 0) {
        ctxSelBegin(ctx);
        ctxCurMoveBack(ctx);
        ctxSelEnd(ctx);
    }

    _ctxRemoveSelections(ctx);
}

void ctxRemoveFwd(Ctx *ctx) {
    if (ctx->_selecting) {
        ctxSelEnd(ctx);
    }

    if (ctx->_sels.len == 0) {
        ctxSelBegin(ctx);
        ctxCurMoveFwd(ctx);
        ctxSelEnd(ctx);
    }

    _ctxRemoveSelections(ctx);
}

void ctxInsertLineAbove(Ctx *ctx) {
    ctxCurMoveToLineStart(ctx);
    ctxInsert(ctx, (const UcdCh8 *)"\n", 1);
    ctxCurMoveBack(ctx);
}

void ctxInsertLineBelow(Ctx *ctx) {
    ctxCurMoveToLineEnd(ctx);
    ctxInsert(ctx, (const UcdCh8 *)"\n", 1);
}

size_t ctxLineCount(const Ctx *ctx) {
    size_t line;
    ctxPosAt(ctx, ctx->_buf.len, &line, NULL);
    return line + 1;
}

static size_t _ctxCurAt(const Ctx *ctx, size_t idx) {
    size_t hi = ctx->cursors.len;
    size_t lo = 0;
    CtxCursor *cursors = ctx->cursors.items;

    while (lo < hi) {
        size_t mid = (lo + hi) / 2;
        if (cursors[mid].idx == idx) {
            return mid;
        } else if (cursors[mid].idx > idx) {
            hi = mid;
        } else {
            lo = mid + 1;
        }

    }

    return lo;
}

static void _ctxCurAddEx(Ctx *ctx, size_t idx, size_t col) {
    size_t curIdx = _ctxCurAt(ctx, idx);
    CtxCursor cursor = { .idx = idx, .baseCol = col, ._selStart = idx };

    if (curIdx >= ctx->cursors.len) {
        arrAppend(&ctx->cursors, cursor);
    // Do not duplicate cursors
    } else if (ctx->cursors.items[curIdx].idx != idx) {
        arrInsert(&ctx->cursors, curIdx, cursor);
    }
}

void ctxCurAdd(Ctx *ctx, size_t idx) {
    size_t col;
    ctxPosAt(ctx, idx, NULL, &col);
    _ctxCurAddEx(ctx, idx, col);
}

void ctxCurRemove(Ctx *ctx, size_t idx) {
    size_t curIdx = _ctxCurAt(ctx, idx);
    if (curIdx < ctx->cursors.len && ctx->cursors.items[curIdx].idx == idx) {
        arrRemove(&ctx->cursors, curIdx);
    }
}

static bool _ctxCurMove(Ctx *ctx, size_t old, size_t new) {
    size_t newCol;
    ctxPosAt(ctx, new, NULL, &newCol);
    return _ctxCurMoveEx(ctx, old, new, newCol);
}

static bool _ctxCurMoveEx(
    Ctx *ctx,
    size_t old,
    size_t new,
    size_t newCol
) {
    size_t oldIdx = _ctxCurAt(ctx, old);
    size_t newIdx = _ctxCurAt(ctx, new);
    CtxCursor *cursors = ctx->cursors.items;

    // If `old` does not exist
    if (oldIdx >= ctx->cursors.len || cursors[oldIdx].idx != old) {
        ctxCurAdd(ctx, new);
        return false;
    }

    if (old == new) {
        return false;
    }

    // If `new` is already a cursor
    if (newIdx < ctx->cursors.len && cursors[newIdx].idx == new) {
        if (new < old) {
            cursors[newIdx]._selStart = nvMax(
                cursors[newIdx]._selStart,
                cursors[oldIdx]._selStart
            );
        } else {
            cursors[newIdx]._selStart = nvMin(
                cursors[newIdx]._selStart,
                cursors[oldIdx]._selStart
            );
        }
        ctxCurRemove(ctx, old);
        return true;
    }

    CtxCursor newCursor = {
        .idx = new,
        .baseCol = newCol,
        ._selStart = cursors[oldIdx]._selStart
    };
    if (oldIdx == newIdx) {
        cursors[oldIdx] = newCursor;
    } else if (oldIdx < newIdx) {
        memmove(
            &cursors[oldIdx],
            &cursors[oldIdx + 1],
            sizeof(*cursors) * (newIdx - oldIdx)
        );
        cursors[newIdx - 1] = newCursor;
    } else {
        memmove(
            &cursors[newIdx + 1],
            &cursors[newIdx],
            sizeof(*cursors) * (oldIdx - newIdx)
        );
        cursors[newIdx] = newCursor;
    }
    return false;
}

void ctxCurMove(Ctx *ctx, size_t old, size_t new) {
    (void)_ctxCurMove(ctx, old, new);
}

void ctxCurMoveLeft(Ctx *ctx) {
    for (size_t i = 0; i < ctx->cursors.len; i++) {
        size_t oldCur = ctx->cursors.items[i].idx;
        ptrdiff_t newCur = ctxLinePrev(ctx, oldCur, NULL);
        if (newCur < 0) {
            continue;
        }
        if (_ctxCurMove(ctx, oldCur, (size_t)newCur)) {
            i--;
        }
    }
}

void ctxCurMoveRight(Ctx *ctx) {
    for (size_t i = 0; i < ctx->cursors.len; i++) {
        // Move from the last cursor to avoid incorrect merging of cursors
        size_t oldCur = ctx->cursors.items[ctx->cursors.len - i - 1].idx;
        if (oldCur == ctx->_buf.len
            || *_ctxBufGet(&ctx->_buf, oldCur) == '\n'
        ) {
            continue;
        }
        ptrdiff_t newCur = ctxNext(ctx, oldCur, NULL);
        if (newCur < 0) {
            newCur = ctx->_buf.len;
        }
        if (_ctxCurMove(ctx, oldCur, (size_t)newCur)) {
            i--;
        }
    }
}

void ctxCurMoveUp(Ctx *ctx) {
    for (size_t i = 0; i < ctx->cursors.len; i++) {
        CtxCursor oldCur = ctx->cursors.items[i];
        size_t oldLine;
        ctxPosAt(ctx, oldCur.idx, &oldLine, NULL);
        ptrdiff_t newCur = ctxIdxAt(ctx, oldLine - 1, oldCur.baseCol, NULL);
        if (newCur < 0) {
            continue;
        }
        if (_ctxCurMoveEx(ctx, oldCur.idx, (size_t)newCur, oldCur.baseCol)) {
            i--;
        }
    }
}

void ctxCurMoveDown(Ctx *ctx) {
    for (size_t i = 0; i < ctx->cursors.len; i++) {
        // Move from the last cursor to avoid incorrect merging of cursors
        CtxCursor oldCur = ctx->cursors.items[ctx->cursors.len - i - 1];
        size_t oldLine;
        ctxPosAt(ctx, oldCur.idx, &oldLine, NULL);
        ptrdiff_t newCur = ctxIdxAt(ctx, oldLine + 1, oldCur.baseCol, NULL);
        if (newCur < 0) {
            continue;
        }
        if (_ctxCurMoveEx(ctx, oldCur.idx, (size_t)newCur, oldCur.baseCol)) {
            i--;
        }
    }
}

void ctxCurMoveFwd(Ctx *ctx) {
    for (size_t i = 0; i < ctx->cursors.len; i++) {
        // Move from the last cursor to avoid incorrect merging of cursors
        size_t oldCur = ctx->cursors.items[ctx->cursors.len - i - 1].idx;
        ptrdiff_t newCur = ctxNext(ctx, oldCur, NULL);
        if (newCur < 0) {
            newCur = ctx->_buf.len;
        }
        if (_ctxCurMove(ctx, oldCur, (size_t)newCur)) {
            i--;
        }
    }
}

void ctxCurMoveBack(Ctx *ctx) {
    for (size_t i = 0; i < ctx->cursors.len; i++) {
        size_t oldCur = ctx->cursors.items[i].idx;
        ptrdiff_t newCur = ctxPrev(ctx, oldCur, NULL);
        if (newCur < 0) {
            continue;
        }
        if (_ctxCurMove(ctx, oldCur, (size_t)newCur)) {
            i--;
        }
    }
}

void ctxCurMoveToLineStart(Ctx *ctx) {
    for (size_t i = 0; i < ctx->cursors.len; i++) {
        size_t oldCur = ctx->cursors.items[i].idx;
        size_t lineNo;
        ctxPosAt(ctx, oldCur, &lineNo, NULL);
        ptrdiff_t newCur = _ctxLineStart(ctx, lineNo);
        if (newCur < 0) {
            continue;
        }
        if (_ctxCurMove(ctx, oldCur, newCur)) {
            i--;
        }
    }
}

void ctxCurMoveToLineEnd(Ctx *ctx) {
    for (size_t i = 0; i < ctx->cursors.len; i++) {
        size_t oldCur = ctx->cursors.items[i].idx;
        size_t lineNo;
        ctxPosAt(ctx, oldCur, &lineNo, NULL);
        ptrdiff_t newCur = _ctxLineEnd(ctx, lineNo);
        if (newCur < 0) {
            continue;
        }
        if (_ctxCurMove(ctx, oldCur, newCur)) {
            i--;
        }
    }
}

void ctxCurMoveToTextStart(Ctx *ctx) {
    for (size_t i = 0; i < ctx->cursors.len; i++) {
        size_t oldCur = ctx->cursors.items[i].idx;
        if (_ctxCurMove(ctx, oldCur, 0)) {
            i--;
        }
    }
}

void ctxCurMoveToTextEnd(Ctx *ctx) {
    size_t newBaseCol;
    ctxPosAt(ctx, ctx->_buf.len, NULL, &newBaseCol);
    for (size_t i = 0; i < ctx->cursors.len; i++) {
        // Move from the last to reduce copying
        size_t oldCur = ctx->cursors.items[ctx->cursors.len - i - 1].idx;
        if (_ctxCurMoveEx(ctx, oldCur, ctx->_buf.len, newBaseCol)) {
            i--;
        }
    }
}

void ctxCurMoveToNextWordStart(Ctx *ctx) {
    for (size_t i = 0; i < ctx->cursors.len; i++) {
        // Move from the last cursor to avoid incorrect merging of cursors
        size_t oldCur = ctx->cursors.items[ctx->cursors.len - i - 1].idx;
        size_t newCur = _ctxFindNextWordStart(ctx, oldCur);
        if (_ctxCurMove(ctx, oldCur, newCur)) {
            i--;
        }
    }
}

void ctxCurMoveToNextWordEnd(Ctx *ctx) {
    for (size_t i = 0; i < ctx->cursors.len; i++) {
        // Move from the last cursor to avoid incorrect merging of cursors
        size_t oldCur = ctx->cursors.items[ctx->cursors.len - i - 1].idx;
        size_t newCur = _ctxFindNextWordEnd(ctx, oldCur);
        if (_ctxCurMove(ctx, oldCur, newCur)) {
            i--;
        }
    }
}

void ctxCurMoveToPrevWordStart(Ctx *ctx) {
    for (size_t i = 0; i < ctx->cursors.len; i++) {
        size_t oldCur = ctx->cursors.items[i].idx;
        size_t newCur = _ctxFindPrevWordStart(ctx, oldCur);
        if (_ctxCurMove(ctx, oldCur, newCur)) {
            i--;
        }
    }
}

void ctxCurMoveToPrevWordEnd(Ctx *ctx) {
    for (size_t i = 0; i < ctx->cursors.len; i++) {
        size_t oldCur = ctx->cursors.items[i].idx;
        size_t newCur = _ctxFindPrevWordEnd(ctx, oldCur);
        if (_ctxCurMove(ctx, oldCur, newCur)) {
            i--;
        }
    }
}

static size_t _lineCountUpperBound(const Ctx *ctx) {
    if (ctx->_refs.len == 0) {
        return ctx->_buf.len;
    } else {
        CtxRef *ref = &ctx->_refs.items[ctx->_refs.len - 1];
        return ref->line + (ctx->_buf.len - ref->idx);
    }
}

void ctxCurMoveToNextParagraph(Ctx *ctx) {
    size_t maxLineNo = _lineCountUpperBound(ctx);
    for (size_t i = 0; i < ctx->cursors.len; i++) {
        size_t oldCur = ctx->cursors.items[ctx->cursors.len - i - 1].idx;
        size_t lineNo;
        ctxPosAt(ctx, oldCur, &lineNo, NULL);
        ptrdiff_t newCur = -1;
        bool skippedBlankLines = false;
        for (;;) {
            ptrdiff_t lineStart = _ctxLineStart(ctx, lineNo);
            if (lineStart < 0) {
                break;
            }
            newCur = _ctxLineEnd(ctx, lineNo);
            assert(newCur >= 0);
            if (newCur - lineStart != 0) {
                skippedBlankLines = true;
            } else if (skippedBlankLines) {
                break;
            }
            lineNo++;
            // Avoid infinite loop
            if (lineNo > maxLineNo) {
                break;
            }
        }

        assert(newCur >= 0);
        if (_ctxCurMove(ctx, oldCur, (size_t)newCur)) {
            i--;
        }
    }
}

void ctxCurMoveToPrevParagraph(Ctx *ctx) {
    for (size_t i = 0; i < ctx->cursors.len; i++) {
        size_t oldCur = ctx->cursors.items[i].idx;
        size_t lineNo;
        ctxPosAt(ctx, oldCur, &lineNo, NULL);
        ptrdiff_t newCur = -1;
        bool skippedBlankLines = false;
        for (;;) {
            ptrdiff_t lineEnd = _ctxLineEnd(ctx, lineNo);
            if (lineEnd < 0) {
                break;
            }
            newCur = _ctxLineStart(ctx, lineNo);
            assert(newCur >= 0);
            if (lineEnd - newCur != 0) {
                skippedBlankLines = true;
            } else if (skippedBlankLines) {
                break;
            }
            if (lineNo == 0) {
                break;
            }
            lineNo--;
        }

        assert(newCur >= 0);
        if (_ctxCurMove(ctx, oldCur, newCur)) {
            i--;
        }
    }
}

static int _selCmp(const void *a, const void *b) {
    const CtxSelection *selA = a;
    const CtxSelection *selB = b;
    ptrdiff_t diff = (ptrdiff_t)selA->startIdx - (ptrdiff_t)selB->startIdx;
    if (diff < 0) {
        return -1;
    } else if (diff > 0) {
        return 1;
    } else {
        return 0;
    }
}

static void _ctxSelJoin(Ctx *ctx) {
    // Append non-empty selections
    for (size_t i = 0; i < ctx->cursors.len; i++) {
        CtxCursor *cursor = &ctx->cursors.items[i];
        CtxSelection sel;
        if (cursor->idx < cursor->_selStart) {
            sel.startIdx = cursor->idx;
            sel.endIdx = cursor->_selStart;
        } else if (cursor->_selStart < cursor->idx) {
            sel.startIdx = cursor->_selStart;
            sel.endIdx = cursor->idx;
        } else {
            continue;
        }
        arrAppend(&ctx->_sels, sel);
    }

    // Sort the selection based on the starting index
    qsort(ctx->_sels.items, ctx->_sels.len, sizeof(*ctx->_sels.items), _selCmp);

    // Join all overlapping or adjacent selections
    for (size_t i = 1; i < ctx->_sels.len; i++) {
        if (ctx->_sels.items[i - 1].endIdx < ctx->_sels.items[i].startIdx) {
            continue;
        }
        if (ctx->_sels.items[i - 1].endIdx < ctx->_sels.items[i].endIdx) {
            ctx->_sels.items[i - 1].endIdx = ctx->_sels.items[i].endIdx;
        }
        arrRemove(&ctx->_sels, i);
        i--;
    }
}

void ctxSelBegin(Ctx *ctx) {
    for (size_t i = 0; i < ctx->cursors.len; i++) {
        ctx->cursors.items[i]._selStart = ctx->cursors.items[i].idx;
    }
    ctx->_selecting = true;
}

void ctxSelEnd(Ctx *ctx) {
    ctx->_selecting = false;
    _ctxSelJoin(ctx);
}

void ctxSelCancel(Ctx *ctx) {
    ctx->_selecting = false;
    arrClear(&ctx->_sels);
}

bool ctxSelIsActive(const Ctx *ctx) {
    return ctx->_selecting;
}

bool ctxSelHas(const Ctx *ctx) {
    return ctx->_selecting || ctx->_sels.len > 0;
}

static void _strAppendBufSpan(
    const CtxBuf *buf,
    Str *str,
    size_t start,
    size_t end
) {
    size_t gapIdx = buf->gapIdx;

    if (gapIdx <= start || gapIdx >= end) {
        strAppendRaw(str, _ctxBufGet(buf, start), end - start);
        return;
    }

    strAppendRaw(str, _ctxBufGet(buf, start), gapIdx - start);
    strAppendRaw(str, _ctxBufGet(buf, gapIdx), end - gapIdx);
}

Str *ctxSelText(Ctx *ctx) {
    if (ctx->_selecting) {
        ctxSelEnd(ctx);
    }

    Str *ret = strNew(0);
    for (size_t i = 0; i < ctx->_sels.len; i++) {
        CtxSelection sel = ctx->_sels.items[i];
        _strAppendBufSpan(&ctx->_buf, ret, sel.startIdx, sel.endIdx);
        if (i + 1 != ctx->_sels.len) {
            strAppendC(ret, "\n");
        }
    }

    return ret;
}
