#ifndef NV_EDITOR_H_
#define NV_EDITOR_H_

#include "nv_screen.h"
#include "nv_context.h"

typedef enum EditorMode {
    EditorMode_Normal,
    EditorMode_Insert,
    EditorMode_SaveDialog
} EditorMode;

typedef struct EditorStrings {
    Str savePrompt;
} EditorStrings;

// The state of the editor.
typedef struct Editor {
    Screen screen;
    EditorMode mode; // Current editor mode.
    Ctx fileCtx;
    Ctx saveDialogCtx;
    EditorStrings strings;
    uint8_t tabStop; // Stop multiple for tab characters.
    bool running; // If the editor is running.
} Editor;

// Global editor variable.
extern Editor g_ed;

// Initialize the editor.
void editorInit(void);
// Deinitialize the editor.
void editorQuit(void);
// Query the size of the terminal and update the editor accordingly.
bool editorUpdateSize(void);

// Handle a key event.
void editorHandleKey(uint32_t key);

// Refresh the editor screen.
bool editorRefresh(void);

// Get the current active context.
Ctx *editorGetActiveCtx(void);

// Save the current file. Fail if no path is set for the file context.
bool editorSaveFile(void);

#endif // !NV_EDITOR_H_
