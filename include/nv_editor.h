#ifndef NV_EDITOR_H_
#define NV_EDITOR_H_

#include "nv_screen.h"
#include "nv_context.h"
#include "nv_buffer.h"

typedef struct EditorStrings {
    Str savePrompt;
} EditorStrings;

// The state of the editor.
typedef struct Editor {
    Screen screen;
    Buf fileBuf; // TODO: multiple file buffers
    Ctx saveDialogCtx;
    EditorStrings strings;
    uint8_t tabStop; // Stop multiple for tab characters.
    bool running; // If the editor is running.
    bool changingName;
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
