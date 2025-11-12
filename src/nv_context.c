#include <string.h>
#include <assert.h>
#include "nv_context.h"
#include "nv_mem.h"
#include "nv_unicode.h"

#ifndef lineRefMaxGap_
#define lineRefMaxGap_ 4096
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

// Get the index of the first character of a line, it may be equal to the length
// of the buffer
static ptrdiff_t ctxLineStart_(const Ctx *ctx, size_t lineNo);
// Get the index of the last characer of a line, it may be equal to the length
// of the buffer
static ptrdiff_t ctxLineEnd_(const Ctx *ctx, size_t lineNo);

// Get the unicode character after `idx`
static UcdCP ctxGetChAfter_(const Ctx *ctx, size_t idx);
// Get the unicode character before `idx`
static UcdCP ctxGetChBefore_(const Ctx *ctx, size_t idx);

static size_t ctxFindNextWordStart_(const Ctx *ctx, size_t idx);
static size_t ctxFindNextWordEnd_(const Ctx *ctx, size_t idx);
static size_t ctxFindPrevWordStart_(const Ctx *ctx, size_t idx);
static size_t ctxFindPrevWordEnd_(const Ctx *ctx, size_t idx);

// Get the index of the cursor at idx or of the cursor on where it should be
// inserted
static size_t ctxCurAt_(const Ctx *ctx, size_t idx);
static void ctxCurAddEx_(Ctx *ctx, size_t idx, size_t col);
// Return `true` if the new cursor already exists
static bool ctxCurReplace_(Ctx *ctx, size_t old, size_t new);
static bool ctxCurReplaceEx_(Ctx *ctx, size_t old, size_t new, size_t newCol);

// Get the line and column at `idx`
static void ctxPosAt_(
    const Ctx *ctx,
    size_t idx,
    size_t *outLine,
    size_t *outCol
);

// Find the index at position `line`, `col`.
// The line must match exactly, the column is the closest to `col`.
static ptrdiff_t ctxIdxAt_(const Ctx *ctx, size_t line, size_t col);

void ctxInit(Ctx *ctx, bool multiline) {
    ctx->_refs = (CtxRefs){ 0 };
    ctx->cursors = (CtxCursors){ 0 };
    ctx->_selects = (CtxSelects){ 0 };

    ctx->_buf = (CtxBuf) {
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
    arrDestroy(&ctx->_refs);
    arrDestroy(&ctx->cursors);
    arrDestroy(&ctx->_selects);

    memFree(ctx->_buf.bytes);

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
        || ucdCh8IsStart(*ctxBufGet_(buf, buf->gapIdx))
    );

    ctxBufShrink_(buf);
}

static void ctxBufSetGapIdx_(CtxBuf *buf, size_t gapIdx) {
    assert(gapIdx <= buf->len);
    assert(
        gapIdx == buf->len
        || ucdCh8IsStart(*ctxBufGet_(buf, gapIdx))
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
static ptrdiff_t ctxGetLineRefBlock_(const Ctx *ctx, size_t lineNo) {
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

static ptrdiff_t ctxLineStart_(const Ctx *ctx, size_t lineNo) {
    if (lineNo == 0) {
        return 0;
    }

    ptrdiff_t refsIdx = ctxGetLineRefBlock_(ctx, lineNo);
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
        if (*ctxBufGet_(&ctx->_buf, i) != '\n') {
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

static ptrdiff_t ctxLineEnd_(const Ctx *ctx, size_t lineNo) {
    ptrdiff_t refsIdx = ctxGetLineRefBlock_(ctx, lineNo + 1);
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
        if (*ctxBufGet_(&ctx->_buf, i) != '\n') {
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

static void ctxPosAt_(
    const Ctx *ctx,
    size_t idx,
    size_t *outLine,
    size_t *outCol
) {
    assert(idx <= ctx->_buf.len);

    size_t refsLen = ctx->_refs.len;
    CtxRef *refs = ctx->_refs.items;

    size_t line = 0;
    size_t startIdx = 0;
    size_t col = 0;

    if (refsLen != 0 && idx >= refs[0].idx) {
        size_t lo = 0;
        size_t hi = refsLen;

        while (lo < hi) {
            size_t mid = (hi + lo) / 2;

            if (refs[mid].idx == idx) {
                lo = mid + 1;
                break;
            } else if (refs[mid].idx >= idx) {
                hi = mid;
            } else {
                lo = mid + 1;
            }
        }

        startIdx = refs[lo - 1].idx;
        line = refs[lo - 1].line;
        col = refs[lo - 1].col;
    }

    for (size_t i = startIdx; i < idx; i++) {
        if (*ctxBufGet_(&ctx->_buf, i) == '\n') {
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

static ptrdiff_t ctxIdxAt_(const Ctx *ctx, size_t line, size_t col) {
    ptrdiff_t refsIdx = ctxGetLineRefBlock_(ctx, line);
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
        if (*ctxBufGet_(&ctx->_buf, i) != '\n') {
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
            return i;
        }
        uint8_t chWidth = ucdCPWidth(cp, ctx->tabStop, refCol);
        if (refCol + chWidth < col) {
            refCol += chWidth;
            continue;
        }
        // If it is closer to the end of the character do another iteration
        if (refCol + chWidth - col < col - refCol) {
            refCol = col;
            continue;
        }
        return i;
    }
    return ctx->_buf.len;
}

ptrdiff_t ctxNext(const Ctx *ctx, ptrdiff_t idx, UcdCP *outCP) {
    if (idx < 0) {
        idx = 0;
    } else if ((size_t)idx < ctx->_buf.len) {
        uint8_t offset = ucdCh8RunLen(*ctxBufGet_(&ctx->_buf, idx));
        // If idx is not on a character boundary find the next one
        if (offset == 0) {
            idx++;
            while (
                idx < ctx->_buf.len
                && !ucdCh8IsStart(*ctxBufGet_(&ctx->_buf, idx))
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
        *outCP = ucdCh8ToCP(ctxBufGet_(&ctx->_buf, idx));
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
    UcdCh8 *chPtr = ctxBufGet_(&ctx->_buf, idx);
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
    ptrdiff_t i = ctxLineStart_(ctx, lineIdx);
    if (i == ctx->_buf.len || i < 0) {
        goto noLine;
    }
    if (*ctxBufGet_(&ctx->_buf, i) == '\n') {
        goto noLine;
    }
    if (outCP) {
        *outCP = ucdCh8ToCP(ctxBufGet_(&ctx->_buf, i));
    }
    return i;

noLine:
    if (outCP) {
        *outCP = -1;
    }
    return -1;
}

ptrdiff_t ctxLinePrevStart(const Ctx *ctx, size_t lineIdx, UcdCP *outCP) {
    ptrdiff_t i = ctxLineEnd_(ctx, lineIdx);
    if (i <= 0) {
        goto noLine;
    }
    i--;
    UcdCh8 *chPtr = ctxBufGet_(&ctx->_buf, i);
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
        idx += ucdCh8RunLen(*ctxBufGet_(&ctx->_buf, idx));
    }

    if ((size_t)idx >= ctx->_buf.len) {
        goto endReached;
    }

    UcdCh8 *chPtr = ctxBufGet_(&ctx->_buf, idx);

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
    UcdCh8 *chPtr = ctxBufGet_(&ctx->_buf, idx);
    while (idx >= 0 && !ucdCh8IsStart(*chPtr)) {
        idx--;
        chPtr = ctxBufGet_(&ctx->_buf, idx);
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

static UcdCP ctxGetChAfter_(const Ctx *ctx, size_t idx) {
    if (idx >= ctx->_buf.len) {
        return -1;
    }
    return ucdCh8ToCP(ctxBufGet_(&ctx->_buf, idx));
}

static UcdCP ctxGetChBefore_(const Ctx *ctx, size_t idx) {
    UcdCP cp;
    (void)ctxPrev(ctx, idx, &cp);
    return cp;
}

static size_t ctxFindNextWordStart_(const Ctx *ctx, size_t idx) {
    if (idx >= ctx->_buf.len) {
        return ctx->_buf.len;
    }
    ptrdiff_t i = idx;
    UcdCP cp = ctxGetChAfter_(ctx, idx);

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

static size_t ctxFindNextWordEnd_(const Ctx *ctx, size_t idx) {
    if (idx >= ctx->_buf.len) {
        return ctx->_buf.len;
    }
    ptrdiff_t i = idx;
    UcdCP cp = ctxGetChAfter_(ctx, idx);

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

static size_t ctxFindPrevWordStart_(const Ctx *ctx, size_t idx) {
    if (idx == 0) {
        return 0;
    }
    ptrdiff_t i = idx;
    UcdCP cp = ctxGetChBefore_(ctx, idx);

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

static size_t ctxFindPrevWordEnd_(const Ctx *ctx, size_t idx) {
    if (idx == 0) {
        return 0;
    }
    ptrdiff_t i = idx;
    UcdCP cp = ctxGetChBefore_(ctx, idx);

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

void ctxAppend(Ctx *ctx, const UcdCh8 *data, size_t len) {
    if (len == 0) {
        return;
    }

    ctx->edited = true;

    size_t lineStart = 0;
    bool ignoreNL = !ctx->multiline;

    CtxBuf *buf = &ctx->_buf;
    size_t initialLen = buf->len;
    uint16_t lastBlockSize = ctx->_refs.len == 0
        ? initialLen
        : initialLen - ctx->_refs.items[ctx->_refs.len - 1].idx;

    ctxBufSetGapIdx_(buf, initialLen);

    for (size_t i = 0; i < len; i++) {
        if (data[i] == '\n' && !ignoreNL) {
            ctxBufInsert_(buf, &data[lineStart], i - lineStart);
            ctxBufInsert_(buf, (const UcdCh8 *)"\n", 1);
            lineStart = i + 1;
        } else if (data[i] == '\r' || data[i] == '\n') {
            ctxBufInsert_(buf, &data[lineStart], i - lineStart);
            lineStart = i + 1;
            continue;
        }

        // Do not insert blocks in the middle of a UTF8 sequence
        if (
            ++lastBlockSize < lineRefMaxGap_
            || (i + 1 < len && !ucdCh8IsStart(data[i + 1]))
        ) {
            continue;
        }

        ctxBufInsert_(buf, &data[lineStart], i - lineStart + 1);
        lineStart = i + 1;
        size_t line, col;
        ctxPosAt_(ctx, ctx->_buf.len, &line, &col);
        arrAppend(
            &ctx->_refs,
            (CtxRef){ .idx = ctx->_buf.len, .line = line, .col = col }
        );
        lastBlockSize = 0;
    }

    if (lineStart < len) {
        ctxBufInsert_(buf, &data[lineStart], len - lineStart);
    }
}

static size_t ctxCurAt_(const Ctx *ctx, size_t idx) {
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

static void ctxCurAddEx_(Ctx *ctx, size_t idx, size_t col) {
    size_t curIdx = ctxCurAt_(ctx, idx);
    CtxCursor cursor = { .idx = idx, .baseCol = col };

    if (curIdx >= ctx->cursors.len) {
        arrAppend(&ctx->cursors, cursor);
    // Do not duplicate cursors
    } else if (ctx->cursors.items[curIdx].idx != idx) {
        arrInsert(&ctx->cursors, curIdx, cursor);
    }
}

void ctxCurAdd(Ctx *ctx, size_t idx) {
    size_t col;
    ctxPosAt_(ctx, idx, NULL, &col);
    ctxCurAddEx_(ctx, idx, col);
}

void ctxCurRemove(Ctx *ctx, size_t idx) {
    size_t curIdx = ctxCurAt_(ctx, idx);
    if (curIdx < ctx->cursors.len && ctx->cursors.items[curIdx].idx == idx) {
        arrRemove(&ctx->cursors, curIdx);
    }
}

static bool ctxCurReplace_(Ctx *ctx, size_t old, size_t new) {
    size_t newCol;
    ctxPosAt_(ctx, new, NULL, &newCol);
    return ctxCurReplaceEx_(ctx, old, new, newCol);
}

static bool ctxCurReplaceEx_(
    Ctx *ctx,
    size_t old,
    size_t new,
    size_t newCol
) {
    size_t oldIdx = ctxCurAt_(ctx, old);
    size_t newIdx = ctxCurAt_(ctx, new);
    CtxCursor *cursors = ctx->cursors.items;

    // If `new` is already a cursor
    if (newIdx < ctx->cursors.len && cursors[newIdx].idx == new) {
        ctxCurRemove(ctx, old);
        return true;
    }

    // If `old` does not exist
    if (oldIdx >= ctx->cursors.len || cursors[oldIdx].idx != old) {
        ctxCurAdd(ctx, new);
        return false;
    }

    CtxCursor newCursor = { .idx = new, .baseCol = newCol };
    if (oldIdx == newIdx) {
        cursors[oldIdx] = newCursor;
    } else if (oldIdx < newIdx) {
        memmove(
            &cursors[oldIdx],
            &cursors[oldIdx + 1],
            sizeof(*cursors) * (newIdx - oldIdx - 1)
        );
        cursors[newIdx - 1] = newCursor;
    } else {
        memmove(
            &cursors[newIdx + 1],
            &cursors[newIdx],
            sizeof(*cursors) * (oldIdx - newIdx - 1)
        );
        cursors[newIdx] = newCursor;
    }
    return false;
}

void ctxCurReplace(Ctx *ctx, size_t old, size_t new) {
    (void)ctxCurReplace_(ctx, old, new);
}

void ctxCurMoveLeft(Ctx *ctx) {
    for (size_t i = 0; i < ctx->cursors.len; i++) {
        size_t oldCur = ctx->cursors.items[i].idx;
        ptrdiff_t newCur = ctxLinePrev(ctx, oldCur, NULL);
        if (newCur < 0) {
            continue;
        }
        if (ctxCurReplace_(ctx, oldCur, (size_t)newCur)) {
            i--;
        }
    }
}

void ctxCurMoveRight(Ctx *ctx) {
    for (size_t i = 0; i < ctx->cursors.len; i++) {
        // Move from the last cursor to avoid incorrect merging of cursors
        size_t oldCur = ctx->cursors.items[ctx->cursors.len - i - 1].idx;
        if (oldCur == ctx->_buf.len
            || *ctxBufGet_(&ctx->_buf, oldCur) == '\n'
        ) {
            continue;
        }
        ptrdiff_t newCur = ctxNext(ctx, oldCur, NULL);
        if (newCur < 0) {
            newCur = ctx->_buf.len;
        }
        if (ctxCurReplace_(ctx, oldCur, (size_t)newCur)) {
            i--;
        }
    }
}

void ctxCurMoveUp(Ctx *ctx) {
    for (size_t i = 0; i < ctx->cursors.len; i++) {
        // Move from the last cursor to avoid incorrect merging of cursors
        CtxCursor oldCur = ctx->cursors.items[ctx->cursors.len - i - 1];
        size_t oldLine;
        ctxPosAt_(ctx, oldCur.idx, &oldLine, NULL);
        ptrdiff_t newCur = ctxIdxAt_(ctx, oldLine + 1, oldCur.baseCol);
        if (newCur < 0) {
            continue;
        }
        if (ctxCurReplaceEx_(ctx, oldCur.idx, (size_t)newCur, oldCur.baseCol)) {
            i--;
        }
    }
}

void ctxCurMoveDown(Ctx *ctx) {
    for (size_t i = 0; i < ctx->cursors.len; i++) {
        // Move from the last cursor to avoid incorrect merging of cursors
        CtxCursor oldCur = ctx->cursors.items[ctx->cursors.len - i - 1];
        size_t oldLine;
        ctxPosAt_(ctx, oldCur.idx, &oldLine, NULL);
        ptrdiff_t newCur = ctxIdxAt_(ctx, oldLine + 1, oldCur.baseCol);
        if (newCur < 0) {
            continue;
        }
        if (ctxCurReplaceEx_(ctx, oldCur.idx, (size_t)newCur, oldCur.baseCol)) {
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
            continue;
        }
        if (ctxCurReplace_(ctx, oldCur, (size_t)newCur)) {
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
        if (ctxCurReplace_(ctx, oldCur, (size_t)newCur)) {
            i--;
        }
    }
}

void ctxCurMoveToLineStart(Ctx *ctx) {
    for (size_t i = 0; i < ctx->cursors.len; i++) {
        size_t oldCur = ctx->cursors.items[i].idx;
        size_t lineNo;
        ctxPosAt_(ctx, oldCur, NULL, &lineNo);
        ptrdiff_t newCur = ctxLineStart_(ctx, lineNo);
        if (newCur < 0) {
            continue;
        }
        if (ctxCurReplace_(ctx, oldCur, newCur)) {
            i--;
        }
    }
}

void ctxCurMoveToLineEnd(Ctx *ctx) {
    for (size_t i = 0; i < ctx->cursors.len; i++) {
        size_t oldCur = ctx->cursors.items[i].idx;
        size_t lineNo;
        ctxPosAt_(ctx, oldCur, &lineNo, NULL);
        ptrdiff_t newCur = ctxLineEnd_(ctx, lineNo);
        if (newCur < 0) {
            continue;
        }
        if (ctxCurReplace_(ctx, oldCur, newCur)) {
            i--;
        }
    }
}

void ctxCurMoveToTextStart(Ctx *ctx) {
    if (ctx->cursors.len == 0) {
        return;
    }
    ctx->cursors.items[0].idx = 0;
    ctx->cursors.items[0].baseCol = 0;
    ctx->cursors.len = 1;
    arrResize(&ctx->cursors, 5);
}

void ctxCurMoveToTextEnd(Ctx *ctx) {
    if (ctx->cursors.len == 0) {
        return;
    }
    ctx->cursors.items[0].idx = ctx->_buf.len;
    ctxPosAt_(ctx, ctx->_buf.len, NULL, &ctx->cursors.items[0].baseCol);
    ctx->cursors.len = 1;
    arrResize(&ctx->cursors, 5);
}

void ctxCurMoveToNextWordStart(Ctx *ctx) {
    for (size_t i = 0; i < ctx->cursors.len; i++) {
        // Move from the last cursor to avoid incorrect merging of cursors
        size_t oldCur = ctx->cursors.items[ctx->cursors.len - i - 1].idx;
        size_t newCur = ctxFindNextWordStart_(ctx, oldCur);
        if (ctxCurReplace_(ctx, oldCur, newCur)) {
            i--;
        }
    }
}

void ctxCurMoveToNextWordEnd(Ctx *ctx) {
    for (size_t i = 0; i < ctx->cursors.len; i++) {
        // Move from the last cursor to avoid incorrect merging of cursors
        size_t oldCur = ctx->cursors.items[ctx->cursors.len - i - 1].idx;
        size_t newCur = ctxFindNextWordEnd_(ctx, oldCur);
        if (ctxCurReplace_(ctx, oldCur, newCur)) {
            i--;
        }
    }
}

void ctxCurMoveToPrevWordStart(Ctx *ctx) {
    for (size_t i = 0; i < ctx->cursors.len; i++) {
        size_t oldCur = ctx->cursors.items[i].idx;
        size_t newCur = ctxFindPrevWordStart_(ctx, oldCur);
        if (ctxCurReplace_(ctx, oldCur, newCur)) {
            i--;
        }
    }
}

void ctxCurMoveToPrevWordEnd(Ctx *ctx) {
    for (size_t i = 0; i < ctx->cursors.len; i++) {
        size_t oldCur = ctx->cursors.items[i].idx;
        size_t newCur = ctxFindPrevWordEnd_(ctx, oldCur);
        if (ctxCurReplace_(ctx, oldCur, newCur)) {
            i--;
        }
    }
}

static size_t lineCountUpperBound_(const Ctx *ctx) {
    if (ctx->_refs.len == 0) {
        return lineRefMaxGap_;
    } else {
        CtxRef *ref = &ctx->_refs.items[ctx->_refs.len - 1];
        return ref->line + (ctx->_buf.len - ref->idx);
    }
}

void ctxCurMoveToNextParagraph(Ctx *ctx) {
    size_t maxLineNo = lineCountUpperBound_(ctx);
    for (size_t i = 0; i < ctx->cursors.len; i++) {
        size_t oldCur = ctx->cursors.items[ctx->cursors.len - i - 1].idx;
        size_t lineNo;
        ctxPosAt_(ctx, oldCur, &lineNo, NULL);
        ptrdiff_t newCur = -1;
        bool skippedBlankLines = false;
        for (;;) {
            ptrdiff_t lineStart = ctxLineStart_(ctx, lineNo);
            if (lineStart < 0) {
                break;
            }
            newCur = ctxLineEnd_(ctx, lineNo);
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
        if (ctxCurReplace_(ctx, oldCur, (size_t)newCur)) {
            i--;
        }
    }
}

void ctxCurMoveToPrevParagraph(Ctx *ctx) {
    for (size_t i = 0; i < ctx->cursors.len; i++) {
        size_t oldCur = ctx->cursors.items[i].idx;
        size_t lineNo;
        ctxPosAt_(ctx, oldCur, &lineNo, NULL);
        ptrdiff_t newCur = -1;
        bool skippedBlankLines = false;
        for (;;) {
            ptrdiff_t lineEnd = ctxLineEnd_(ctx, lineNo);
            if (lineEnd < 0) {
                break;
            }
            newCur = ctxLineStart_(ctx, lineNo);
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
        if (ctxCurReplace_(ctx, oldCur, newCur)) {
            i--;
        }
    }
}
