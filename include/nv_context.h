#ifndef NV_CONTEXT_H_
#define NV_CONTEXT_H_

#include "nv_array.h"
#include "nv_string.h"

// Context mode
typedef enum CtxMode {
    CtxMode_Normal,
    CtxMode_Insert
} CtxMode;

typedef Arr(size_t) CtxLineRef;
typedef Arr(size_t) CtxCursors;
typedef Arr(size_t) CtxSelects;

typedef struct CtxBuf {
    UcdCh8 *bytes;
    size_t len;
    size_t cap;
    size_t gapIdx;
} CtxBuf;

// Editing context.
typedef struct Ctx {
    CtxLineRef m_lineRef;
    CtxCursors m_cursors;
    CtxSelects m_selects;
    CtxBuf m_buf;
    CtxMode mode;
    bool edited;
    bool multiline;
    uint8_t tabStop;
} Ctx;

/******************** Initialization and deinitialization *********************/

// Initialize a line context.
void ctxInit(Ctx *ctx, bool multiline);
// Destroy a context.
void ctxDestroy(Ctx *ctx);

/****************************** Cursor movement *******************************/

// Move the cursor by `dx` characters on the current line.
void ctxMoveCurX(Ctx *ctx, ptrdiff_t dx);
// Move the cursor by `dy` lines.
void ctxMoveCurY(Ctx *ctx, ptrdiff_t dy);
// Move the cursor by `diffIdx` characters across multiple lines.
void ctxMoveCurIdx(Ctx *ctx, ptrdiff_t diffIdx);

// Move the cursor to the start of the line
void ctxMoveCurLineStart(Ctx *ctx);
// Move the cursor to the end of the line
void ctxMoveCurLineEnd(Ctx *ctx);
// Move the cursor to the start of the file
void ctxMoveCurFileStart(Ctx *ctx);
// Move the cursor to the end of the file
void ctxMoveCurFileEnd(Ctx *ctx);

// Move to the beginning of the next word.
void ctxMoveCurWordStartF(Ctx *ctx);
// Move to the end of the word.
void ctxMoveCurWordEndF(Ctx *ctx);
// Move to the beginning of the word.
void ctxMoveCurWordStartB(Ctx *ctx);
// Move to the start of the previous word.
void ctxMoveCurWordEndB(Ctx *ctx);

// Move to the next blank linke.
void ctxMoveCurParagraphF(Ctx *ctx);
// Move to the previous blank line.
void ctxMoveCurParagraphB(Ctx *ctx);

/********************************** Editing ***********************************/

// Write in a context.
void ctxInsert(Ctx *ctx, const UcdCh8 *data, size_t len);
// Append to a context, does not change the cursor position.
void ctxAppend(Ctx *ctx, const UcdCh8 *data, size_t len);
// Insert a codepoint. Any invalid codepoint is ignored.
void ctxInsertCP(Ctx *ctx, UcdCP cp);
// Remove the cahracter before the cursor.
void ctxRemoveBack(Ctx *ctx);
// Remove the character after the cursor.
void ctxRemoveForeward(Ctx *ctx);

/*********************************** Other ************************************/

// Get the number of lines in the text of a context.
size_t ctxLineCount(const Ctx *ctx);
// Get the length of a line in characters.
size_t ctxLineLen(const Ctx *ctx, size_t lineIdx);
// Get the number of cursors in the context
size_t ctxCursorCount(const Ctx *ctx);

// Get the content of the context as a string view.
// The view is valid until the text is edited.
StrView *ctxGetContent(Ctx *ctx);

// Iterate over the whole content of the context.
// Use `idx == -1` to begin iterating.
// Pass the returned value as `idx` to continue iterating.
// The iteration ends once the return value is `-1`.
// `idx` is the index of the first byte of the character in the text.
ptrdiff_t ctxIterNext(const Ctx *ctx, ptrdiff_t idx, UcdCP *outCP);
// Iterate over the whole content of the context from the end.
// Use `idx == -1` to begin iterating.
// Pass the returned value as `idx` to continue iterating.
// The iteration ends once the return value is `-1`.
// `idx` is the index of the first byte of the character in the text.
ptrdiff_t ctxIterPrev(const Ctx *ctx, ptrdiff_t idx, UcdCP *outCP);
// Start iterating over one line of the context.
// Pass the returned value as `idx` to `ctxLineIterNext` to continue iterating.
// If the return value is `-1` there is nothing to iterate.
// `idx` is the index of the first byte of the character in the text.
ptrdiff_t ctxLineIterNextStart(const Ctx *ctx, size_t lineIdx, UcdCP *outCP);
// Start iterating over one line of the context from the end.
// Pass the returned value as `idx` to `ctxLineIterPrev` to continue iterating.
// If the return value is `-1` there is nothing to iterate.
// `idx` is the index of the first byte of the character in the text.
ptrdiff_t ctxLineIterPrevStart(const Ctx *ctx, size_t lineIdx, UcdCP *outCP);
// Continue iterating over one line of the context.
// Pass the returned value as `idx` to continue iterating.
// The iteration ends once the return value is `-1`.
// `idx` is the index of the first byte of the character in the text.
ptrdiff_t ctxLineIterNext(const Ctx *ctx, ptrdiff_t idx, UcdCP *outCP);
// Continue iterating over one line of the context from the end.
// Pass the returned value as `idx` to continue iterating.
// The iteration ends once the return value is `-1`.
// `idx` is the index of the first byte of the character in the text.
ptrdiff_t ctxLineIterPrev(const Ctx *ctx, ptrdiff_t idx, UcdCP *outCP);

#endif // NV_CONTEXT_H_
