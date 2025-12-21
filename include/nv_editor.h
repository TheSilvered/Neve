#ifndef NV_EDITOR_H_
#define NV_EDITOR_H_

#include "nv_screen.h"
#include "nv_buffer.h"
#include "nv_tui.h"

typedef struct EditorStrings {
    Str savePrompt;
    Str noFilePath;
} EditorStrings;

// The state of the editor.
typedef struct Editor {
    Screen screen;
    BufMap buffers;
    UIBufPanel bufPanel;
    EditorStrings strings;
    bool running;
} Editor;

// Global editor variable.
extern Editor g_ed;

// Initialize the editor.
void editorInit(void);
// Deinitialize the editor.
void editorQuit(void);
// Query the size of the terminal and update the editor accordingly.
bool editorUpdateSize(void);

// Handle a key press.
void editorHandleKey(int32_t key);
// Refresh the editor screen.
bool editorRefresh(void);

// Open a file or create a new buffer with the path.
bool editorOpen(const char *path);
// Create a new file.
void editorNewBuf(void);

#endif // !NV_EDITOR_H_
