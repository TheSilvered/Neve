#ifndef NV_GAPBUFFER_H_
#define NV_GAPBUFFER_H_

#include "nv_unicode.h"

// Gap buffer data structure. Init with `(GBuf) { 0 }`.
// Can be casted to a `GBuf*` to `StrView *` is valid after `gBufUnite` and
// until `gBufInsert`, `gBufRemove`  or `gBufSetGapIdx`.
typedef struct GBuf {
    UcdCh8 *bytes;
    size_t len;
    size_t cap;
    size_t gapIdx;
} GBuf;

// Get a character from a gap buffer.
UcdCh8 gBufGet(const GBuf *buf, size_t idx);
// Get the pointer to a character in the gap buffer.
// The pointer is valid as long as the buffer is not edited.
UcdCh8 *gBufGetPtr(const GBuf *buf, size_t idx);
// Insert data into the gap buffer. The data is inserted at `gapIdx` and
// `gapIdx` itself is added after the newly inserted data.
void gBufInsert(GBuf *buf, const UcdCh8 *text, size_t len);
// Remove `len` bytes before `gapIdx` and move `gapIdx` accordingly.
void gBufRemove(GBuf *buf, size_t len);
// Change the position of `gapIdx`
void gBufSetGapIdx(GBuf *buf, size_t gapIdx);
// Make the text in the gap buffer contiguous (puts `gapIdx` at the end).
void gBufUnite(GBuf *buf);

ptrdiff_t gBufNext(const GBuf *buf, ptrdiff_t idx, UcdCP *outCP);
ptrdiff_t gBufPrev(const GBuf *buf, ptrdiff_t idx, UcdCP *outCP);

#endif // !NV_GAPBUFFER_H_
