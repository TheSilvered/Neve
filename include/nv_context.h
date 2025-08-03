#ifndef NV_CONTEXT_H_
#define NV_CONTEXT_H_

#include "nv_file.h"
#include "nv_string.h"

// Editing context kind.
typedef enum CtxKind {
    CtxKind_File,
    CtxKind_Line
} CtxKind;

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
    CtxKind kind;
    CtxWindow win;
    CtxCursor cur;
    CtxText text;
    Str path;
    bool edited;
} Ctx;

/******************** Initialization and deinitialization *********************/

// Initialize a line context.
void ctxInitLine(Ctx *ctx);
// Initialize a context for a new file.
void ctxInitNewFile(Ctx *ctx, const char *path);
// Initialize a context with the contents of `file`.
bool ctxInitFromFile(Ctx *ctx, File *file);
// Write the contents of a context to a file.
bool ctxWriteToFile(Ctx *ctx, File *file);
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
// Set the save path of the context.
void ctxSetPath(Ctx *ctx, StrView *path);
// Get the number of lines in the text of a context.
size_t ctxLineCount(const Ctx *ctx);
// Get a line of the context as a string view.
StrView ctxGetLine(const Ctx *ctx, size_t lineIdx);

#endif // NV_CONTEXT_H_
