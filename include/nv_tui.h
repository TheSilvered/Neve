#ifndef NV_TUI_H_
#define NV_TUI_H_

#include <stdint.h>
#include "nv_buffer.h"
#include "nv_term.h"

/*
|--------------------------------------------------------------------------------|
|                 |[tab1         x][tab2         x][tab3         x]              |
|                 |                                                              |
|                 |                                                              |
|                 |                                                              |
|                 |                                                              |
|                 |                                                              |
|                 |                                                              |
|                 |                                                              |
|                 |                                                              |
|                 |        /-------Popup Window-------\                          |
|    Side bar     |        |                          |                          |
|                 |        |                          |                          |
|                 |        |      [Yes] [No] [Cancel] |                          |
|                 |        \--------------------------/                          |
|                 |                                                              |
|                 |                                                              |
|                 |                                                              |
|                 |                                                              |
|                 |                                                              |
|                 |--------------------------------------------------------------|
|                 |                           Info box                           |
|                 |                                                              |
|################################## Status Bar ##################################|
|--------------------------------------------------------------------------------|
*/

typedef bool (*UIKeyHandler)(TermKey key);

typedef enum UIPanelKind {
    UIPanelKind_HSplit,
    UIPanelKind_VSplit,
    UIPanelKind_Buf,
    UIPanelKind_Tabs
} UIPanelKind;

typedef struct UIPanel {
    UIPanelKind kind;
    int16_t x, y;
    uint16_t w, h;
    UIKeyHandler _keyHandler;
} UIPanel;

typedef struct UIPanelHSplit {
    UIPanel panel;
    UIPanel *left, *right;
} UIPanelHSplit;

typedef struct UIPanelVSplit {
    UIPanel panel;
    UIPanel *top, *bottom;
} UIPanelVSplit;

typedef struct UIPanelBuf {
    UIPanel panel;
    Buf *buf;
} UIPanelBuf;

UIPanel *uiPanelBufNew(Buf *buf);

#endif // !NV_TUI_H_

