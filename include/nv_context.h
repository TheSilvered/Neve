#ifndef NV_CONTEXT_H_
#define NV_CONTEXT_H_

#include "nv_file.h"
#include "nv_string.h"

// Context mode
typedef enum CtxMode {
    CtxMode_Normal,
    CtxMode_Insert
} CtxMode;

// Window positition (scroll x and y) and size.
typedef struct CtxWindow {
    size_t x, y;
    uint16_t termX, termY;
    uint16_t w, h;
} CtxWindow;

// Cursor position.
typedef struct CtxCursor {
    size_t x, y, baseX, idx;
} CtxCursor;

// Text content of an editing context.
typedef struct CtxText {
    UcdCh8 *buf;
    size_t bufLen, bufCap;
    size_t *lines;
    size_t linesLen, linesCap;
} CtxText;

// Editing context.
typedef struct Ctx {
    CtxWindow win;
    CtxCursor cur;
    CtxText text;
    CtxMode mode;
    bool edited;
    bool multiline;
} Ctx;

/******************** Initialization and deinitialization *********************/

// Initialize a line context.
void ctxInit(Ctx *ctx, bool multiline);
// Destroy a context.
void ctxDestroy(Ctx *ctx);

/****************************** Cursor movement *******************************/

// Get the position of the cursor as it appears on the terminal.
void ctxGetCurTermPos(const Ctx *ctx, uint16_t *outCol, uint16_t *outRow);
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

/********************************** Editing ***********************************/

// Write in a context.
void ctxInsert(Ctx *ctx, const UcdCh8 *data, size_t len);
// Insert a codepoint. Any invalid codepoint is ignored.
void ctxInsertCP(Ctx *ctx, UcdCP cp);
// Remove the cahracter before the cursor.
void ctxRemoveBack(Ctx *ctx);
// Remove the character after the cursor.
void ctxRemoveForeward(Ctx *ctx);

/*********************************** Other ************************************/

// Set the size of the window.
void ctxSetWinSize(Ctx *ctx, uint16_t width, uint16_t height);
// Get the number of lines in the text of a context.
size_t ctxLineCount(const Ctx *ctx);
// Get a line of the context as a string view.
StrView ctxGetLine(const Ctx *ctx, size_t lineIdx);

#endif // NV_CONTEXT_H_
