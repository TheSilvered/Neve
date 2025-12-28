#ifndef NV_EDITOR_H_
#define NV_EDITOR_H_

#include "nv_array.h"
#include "nv_buffer.h"
#include "nv_command.h"
#include "nv_screen.h"
#include "nv_tui.h"

// The state of the editor.
typedef struct Editor {
    Screen screen;
    uint64_t lastUpdate;
    BufMap buffers;
    CmdMap commands;
    CmdResult *cmdResult;
    UI ui;
    bool running;
    bool runningCommand;
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
// Open the command palette.
void editorOpenCommandPalette(void);

#endif // !NV_EDITOR_H_
