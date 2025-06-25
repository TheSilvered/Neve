#ifndef NV_EDITOR_H_
#define NV_EDITOR_H_

#include "nv_string.h"

typedef struct Editor {
    size_t curX, curY;
    size_t rows, cols;
    size_t prevRows, prevCols;
    Str screenBuf;
} Editor;

extern Editor g_ed;

void editorInit(Editor *ed);
void editorQuit(Editor *ed);
bool editorDrawBegin(Editor *ed);
bool editorDraw(Editor *ed, const char *buf, size_t len);
bool editorDrawFmt(Editor *ed, const char *fmt, ...);
bool editorDrawEnd(Editor *ed);
bool editorRowsChanged(Editor *ed);
bool editorColsChanged(Editor *ed);

#endif // !NV_EDITOR_H_
