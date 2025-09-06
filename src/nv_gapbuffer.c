#include <assert.h>
#include <string.h>
#include "nv_gapbuffer.h"
#include "nv_mem.h"

UcdCh8 gBufGet(const GBuf *buf, size_t idx) {
    assert(idx < buf->len);
    if (idx >= buf->gapIdx) {
        size_t gapSize = buf->cap - buf->len;
        idx += gapSize;
    }
    return buf->bytes[idx];
}

UcdCh8 *gBufGetPtr(const GBuf *buf, size_t idx) {
    assert(idx < buf->len);
    if (idx >= buf->gapIdx) {
        size_t gapSize = buf->cap - buf->len;
        idx += gapSize;
    }
    return &buf->bytes[idx];
}

void gBufReserve_(GBuf *buf, size_t amount) {
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

void gBufShrink_(GBuf *buf) {
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

void gBufInsert(GBuf *buf, const UcdCh8 *text, size_t len) {
    gBufReserve_(buf, len);
    memcpy(buf->bytes + buf->gapIdx, text, len * sizeof(*text));
    buf->gapIdx += len;
    buf->len += len;
}

void gBufRemove(GBuf *buf, size_t len) {
    if (len > buf->gapIdx) {
        len = buf->gapIdx;
    }

    buf->gapIdx -= len;
    buf->len -= len;

    assert(buf->gapIdx == buf->len || ucdCh8CPLen(gBufGet(buf, buf->gapIdx)));

    gBufShrink_(buf);
}

void gBufSetGapIdx(GBuf *buf, size_t gapIdx) {
    assert(gapIdx <= buf->len);
    assert(gapIdx == buf->len || ucdCh8CPLen(gBufGet(buf, gapIdx)));
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

void gBufUnite(GBuf *buf) {
    gBufSetGapIdx(buf, buf->len);
}

ptrdiff_t gBufNext(const GBuf *buf, ptrdiff_t idx, UcdCP *outCP) {
    if (idx >= (ptrdiff_t)buf->len) {
        goto endReached;
    } else if (idx < 0) {
        idx = 0;
    } else {
        idx += ucdCh8RunLen(gBufGet(buf, idx));
    }

    if (idx >= (ptrdiff_t)buf->len) {
        goto endReached;
    }
    size_t runLen = ucdCh8RunLen(gBufGet(buf, idx));

    if (runLen == 0 || idx + runLen > buf->len) {
        goto endReached;
    }

    if (outCP != NULL) {
        if ((size_t)idx < buf->gapIdx) {
            *outCP = ucdCh8ToCP(buf->bytes + idx);
        } else {
            size_t gapSize = buf->cap - buf->len;
            *outCP = ucdCh8ToCP(buf->bytes + idx + gapSize);
        }
    }
    return idx;

endReached:
    if (outCP != NULL) {
        *outCP = -1;
    }
    return -1;
}

ptrdiff_t gBufPrev(const GBuf *buf, ptrdiff_t idx, UcdCP *outCP) {
    if (idx == 0) {
        goto endReached;
    } else if (idx < 0) {
        idx = buf->len;
    }
    idx--;
    while (idx >= 0 && ucdCh8RunLen(gBufGet(buf, idx)) == 0) {
        idx--;
    }
    if (idx < 0) {
        goto endReached;
    }
    size_t runLen = ucdCh8RunLen(gBufGet(buf, idx));
    if (runLen == 0 || idx + runLen > buf->len) {
        goto endReached;
    }

    if (outCP != NULL) {
        if ((size_t)idx < buf->gapIdx) {
            *outCP = ucdCh8ToCP(buf->bytes + idx);
        } else {
            size_t gapSize = buf->cap - buf->len;
            *outCP = ucdCh8ToCP(buf->bytes + idx + gapSize);
        }
    }
    return idx;

endReached:
    if (outCP != NULL) {
        *outCP = -1;
    }
    return -1;
}
