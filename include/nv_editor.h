#ifndef NV_EDITOR_H_
#define NV_EDITOR_H_

#include "nv_file.h"
#include "nv_string.h"

// A terminal row.
typedef struct Row {
    Str buf;
    bool changed;
} Row;

// The state of the editor.
typedef struct Editor {
    uint16_t rows, cols; // Terminal resolution.
    Row *rowBuffers; // Rows array (length == .rows)
    Str screenBuf; // Buffer for screen printing.
    File file; // Opened file.
    size_t curX, curY; // Position of the cursor.
    size_t fileCurIdx; // Cursor position in the file.
    size_t fileLineOffset; // Vertical scrolling.
    bool running; // If the editor is running.
} Editor;

// Global editor variable.
extern Editor g_ed;

// Initialize an editor.
void editorInit(Editor *ed);
// Deinitialize an editor.
void editorQuit(Editor *ed);
// Query the size of the terminal and update the editor accordingly.
bool editorUpdateSize(Editor *ed, bool *outRowsChanged, bool *outColsChanged);
// Queue content to be drawn on row `rowIdx`.
// Each call before `editorDrawEnd` appends the contents of buf.
// After `editorDrawEnd` the underlying buffer is cleared and the screen is
// updated only if new content is added to the line.
bool editorDraw(Editor *ed, uint16_t rowIdx, const char *buf, size_t len);
// The same as `editorDraw` but with a printf-style format.
bool editorDrawFmt(Editor *ed, uint16_t rowIdx, const char *fmt, ...);
// Finish drawing and update the screen.
// Only edited lines are updated.
bool editorDrawEnd(Editor *ed);
// Move the cursor by `dx` columns and `dy` rows.
void editorMoveCursor(Editor *ed, ptrdiff_t dx, ptrdiff_t dy);

#endif // !NV_EDITOR_H_
