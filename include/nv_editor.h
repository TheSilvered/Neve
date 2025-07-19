#ifndef NV_EDITOR_H_
#define NV_EDITOR_H_

#include "nv_file.h"
#include "nv_string.h"

typedef struct Row {
    Str buf;
    bool changed;
} Row;

typedef struct Editor {
    uint16_t curX, curY;
    uint16_t rows, cols;
    Row *rowBuffers;
    Str screenBuf;
    File file;
} Editor;

extern Editor g_ed;

void editorInit(Editor *ed);
void editorQuit(Editor *ed);
bool editorUpdateSize(Editor *ed, bool *outRowsChanged, bool *outColsChanged);
bool editorDraw(Editor *ed, uint16_t rowIdx, const char *buf, size_t len);
bool editorDrawFmt(Editor *ed, uint16_t rowIdx, const char *fmt, ...);
bool editorDrawEnd(Editor *ed);

#endif // !NV_EDITOR_H_
