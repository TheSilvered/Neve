#ifndef NV_CONTEXT_H_
#define NV_CONTEXT_H_

#include "nv_array.h"
#include "nv_string.h"

typedef struct CtxRef {
    size_t idx, line, col;
} CtxRef;

typedef struct CtxCursor {
    size_t idx, baseCol;
    size_t selStart;
} CtxCursor;

typedef struct CtxSelection {
    size_t startIdx, endIdx;
} CtxSelection;

typedef Arr(CtxRef) CtxRefs;
typedef Arr(CtxCursor) CtxCursors;
typedef Arr(CtxSelection) CtxSelections;

typedef struct CtxBuf {
    UcdCh8 *bytes;
    size_t len;
    size_t cap;
    size_t gapIdx;
} CtxBuf;

// Editing context.
typedef struct Ctx {
    CtxRefs _refs;
    CtxSelections _sels;
    CtxBuf _buf;
    CtxCursors cursors;
    bool _selecting;
    bool edited;
    bool multiline;
    uint8_t tabStop;
} Ctx;

/******************** Initialization and deinitialization *********************/

// Initialize a line context.
void ctxInit(Ctx *ctx, bool multiline);
// Destroy a context.
void ctxDestroy(Ctx *ctx);

/* Iteration */

// Iterate over the whole content of the context.
// Use `idx == -1` to begin iterating.
// Pass the returned value as `idx` to continue iterating.
// The iteration ends once the return value is `-1`.
// `idx` is the index of the first byte of the character in the text.
ptrdiff_t ctxNext(const Ctx *ctx, ptrdiff_t idx, UcdCP *outCP);
// Iterate over the whole content of the context from the end.
// Use `idx == -1` to begin iterating.
// Pass the returned value as `idx` to continue iterating.
// The iteration ends once the return value is `-1`.
// `idx` is the index of the first byte of the character in the text.
ptrdiff_t ctxPrev(const Ctx *ctx, ptrdiff_t idx, UcdCP *outCP);
// Start iterating over one line of the context.
// Pass the returned value as `idx` to `ctxLineIterNext` to continue iterating.
// If the return value is `-1` there is nothing to iterate.
// `idx` is the index of the first byte of the character in the text.
ptrdiff_t ctxLineNextStart(const Ctx *ctx, size_t lineIdx, UcdCP *outCP);
// Start iterating over one line of the context from the end.
// Pass the returned value as `idx` to `ctxLineIterPrev` to continue iterating.
// If the return value is `-1` there is nothing to iterate.
// `idx` is the index of the first byte of the character in the text.
ptrdiff_t ctxLinePrevStart(const Ctx *ctx, size_t lineIdx, UcdCP *outCP);
// Continue iterating over one line of the context.
// Pass the returned value as `idx` to continue iterating.
// The iteration ends once the return value is `-1`.
// `idx` is the index of the first byte of the character in the text.
ptrdiff_t ctxLineNext(const Ctx *ctx, ptrdiff_t idx, UcdCP *outCP);
// Continue iterating over one line of the context from the end.
// Pass the returned value as `idx` to continue iterating.
// The iteration ends once the return value is `-1`.
// `idx` is the index of the first byte of the character in the text.
ptrdiff_t ctxLinePrev(const Ctx *ctx, ptrdiff_t idx, UcdCP *outCP);

/***************************** Cursor management ******************************/

// Add a cursor at 'idx' in the file
void ctxCurAdd(Ctx *ctx, size_t idx);
// Remove the cursor at 'idx' in the file if it exists
void ctxCurRemove(Ctx *ctx, size_t idx);
// Replace the cursor at 'old' with the cursor at 'new'
void ctxCurMove(Ctx *ctx, size_t old, size_t new);

// Move to the previous character in the line
void ctxCurMoveLeft(Ctx *ctx);
// Move to the next character in the line
void ctxCurMoveRight(Ctx *ctx);
// Move to the previous line
void ctxCurMoveUp(Ctx *ctx);
// Move to the next line
void ctxCurMoveDown(Ctx *ctx);
// Move to the next character
void ctxCurMoveFwd(Ctx *ctx);
// Move to the previous character
void ctxCurMoveBack(Ctx *ctx);

// Move the cursor to the start of the line
void ctxCurMoveToLineStart(Ctx *ctx);
// Move the cursor to the end of the line
void ctxCurMoveToLineEnd(Ctx *ctx);
// Move the cursor to the start of the text
void ctxCurMoveToTextStart(Ctx *ctx);
// Move the cursor to the end of the text
void ctxCurMoveToTextEnd(Ctx *ctx);

// Move to the beginning of the next word.
void ctxCurMoveToNextWordStart(Ctx *ctx);
// Move to the end of the word.
void ctxCurMoveToNextWordEnd(Ctx *ctx);
// Move to the beginning of the word.
void ctxCurMoveToPrevWordStart(Ctx *ctx);
// Move to the end of the previous word.
void ctxCurMoveToPrevWordEnd(Ctx *ctx);

// Move to the next blank linke.
void ctxCurMoveToNextParagraph(Ctx *ctx);
// Move to the previous blank line.
void ctxCurMoveToPrevParagraph(Ctx *ctx);

/********************************* Selecting **********************************/

// Begin a selection at the position of each cursor.
void ctxSelBegin(Ctx *ctx);
// Stops selecting.
void ctxSelEnd(Ctx *ctx);
// Check if selection is active.
bool ctxSelIsActive(const Ctx *ctx);
// Check if there are any selections in the context.
bool ctxSelHas(const Ctx *ctx);
// Get selected text. Each selection is separated by a line feed character.
// Calling this function forces the selection to end.
Str *ctxSelText(Ctx *ctx);

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
ptrdiff_t ctxNext(const Ctx *ctx, ptrdiff_t idx, UcdCP *outCP);
// Iterate over the whole content of the context from the end.
// Use `idx == -1` to begin iterating.
// Pass the returned value as `idx` to continue iterating.
// The iteration ends once the return value is `-1`.
// `idx` is the index of the first byte of the character in the text.
ptrdiff_t ctxPrev(const Ctx *ctx, ptrdiff_t idx, UcdCP *outCP);
// Start iterating over one line of the context.
// Pass the returned value as `idx` to `ctxLineIterNext` to continue iterating.
// If the return value is `-1` there is nothing to iterate.
// `idx` is the index of the first byte of the character in the text.
ptrdiff_t ctxLineNextStart(const Ctx *ctx, size_t lineIdx, UcdCP *outCP);
// Start iterating over one line of the context from the end.
// Pass the returned value as `idx` to `ctxLineIterPrev` to continue iterating.
// If the return value is `-1` there is nothing to iterate.
// `idx` is the index of the first byte of the character in the text.
ptrdiff_t ctxLinePrevStart(const Ctx *ctx, size_t lineIdx, UcdCP *outCP);
// Continue iterating over one line of the context.
// Pass the returned value as `idx` to continue iterating.
// The iteration ends once the return value is `-1`.
// `idx` is the index of the first byte of the character in the text.
ptrdiff_t ctxLineNext(const Ctx *ctx, ptrdiff_t idx, UcdCP *outCP);
// Continue iterating over one line of the context from the end.
// Pass the returned value as `idx` to continue iterating.
// The iteration ends once the return value is `-1`.
// `idx` is the index of the first byte of the character in the text.
ptrdiff_t ctxLinePrev(const Ctx *ctx, ptrdiff_t idx, UcdCP *outCP);

#endif // NV_CONTEXT_H_
