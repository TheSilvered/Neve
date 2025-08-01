#ifndef NV_EDITOR_H_
#define NV_EDITOR_H_

#include "nv_file.h"
#include "nv_string.h"

typedef enum EditorMode {
    EditorMode_Normal,
    EditorMode_Insert
} EditorMode;

// A terminal row.
typedef struct EditorRow {
    Str buf;
    bool changed;
} EditorRow;

// The state of the editor.
typedef struct Editor {
    EditorRow *rowBuffers; // Rows array (length == .rows)
    uint16_t rows, cols; // Terminal resolution.
    uint16_t viewboxW, viewboxH; // Viewbox size.
    Str screenBuf; // Buffer for screen printing.
    File file; // Opened file.
    size_t curX, curY; // Position of the cursor.
    size_t baseCurX; // Column the cursor goes to if possible.
    size_t fileCurIdx; // Cursor position in the file.
    size_t scrollX, scrollY; // Scrolling.
    uint8_t tabStop; // Stop multiple for tab characters.
    EditorMode mode; // Current editor mode.
    bool running; // If the editor is running.
} Editor;

// Global editor variable.
extern Editor g_ed;

// Initialize an editor.
void editorInit(Editor *ed);
// Deinitialize an editor.
void editorQuit(Editor *ed);
// Query the size of the terminal and update the editor accordingly.
bool editorUpdateSize(Editor *ed);
// Set the size and update the viewbox.
void editorSetViewboxSize(Editor *ed, uint16_t width, uint16_t height);
// Queue content to be drawn on row `rowIdx`.
// Each call before `editorDrawEnd` appends the contents of buf.
// After `editorDrawEnd` the underlying buffer is cleared and the screen is
// updated only if new content is added to the line.
bool editorDraw(Editor *ed, uint16_t rowIdx, const UcdCh8 *buf, size_t len);
// The same as `editorDraw` but with a printf-style format.
bool editorDrawFmt(Editor *ed, uint16_t rowIdx, const char *fmt, ...);
// Finish drawing and update the screen.
// Only edited lines are updated.
bool editorDrawEnd(Editor *ed);
// Move the cursor by `dx` characters in the current line.
void editorMoveCursorX(Editor *ed, ptrdiff_t dx);
// Move the cursor by `dy` lines.
void editorMoveCursorY(Editor *ed, ptrdiff_t dy);
// Move the cursor by `diffIdx` characters.
void editorMoveCursorIdx(Editor *ed, ptrdiff_t diffIdx);

#endif // !NV_EDITOR_H_
