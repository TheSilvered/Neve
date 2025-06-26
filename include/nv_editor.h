#ifndef NV_EDITOR_H_
#define NV_EDITOR_H_

#include "nv_string.h"

typedef struct Editor {
    size_t curX, curY;
    size_t rows, cols;
    Str screenBuf;
} Editor;

extern Editor g_ed;

void editorInit(Editor *ed);
void editorQuit(Editor *ed);
bool editorUpdateSize(Editor *ed, bool *outRowsChanged, bool *outColsChanged);
bool editorDraw(Editor *ed, const char *buf, size_t len);
bool editorDrawFmt(Editor *ed, const char *fmt, ...);
bool editorDrawEnd(Editor *ed);

#endif // !NV_EDITOR_H_
