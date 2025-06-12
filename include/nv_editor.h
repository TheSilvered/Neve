#ifndef NV_EDITOR_H_
#define NV_EDITOR_H_

#include "nv_string.h"

typedef struct Editor {
    size_t curX, curY;
    Str screenBuf;
} Editor;

extern Editor g_ed;

void editorInit(Editor *ed);
void editorQuit(Editor *ed);
bool editorDraw(Editor *ed, const char *buf, size_t len);
bool editorFlipScreen(Editor *ed);

#endif // !NV_EDITOR_H_
